// =====================================================================
//  CNetworkException.h                  (C) Hotline Communications 1999
// =====================================================================
// Network problems

#ifndef _H_CNetworkException_
#define _H_CNetworkException_

#include "CException.h"

#if PRAGMA_ONCE
	#pragma once
#endif				   
#include "CNetAddress.h"

HL_Begin_Namespace_BigRedH

class CNetworkException : public CException
{
	public:
		enum ETask 
		{ 
			eResolvingAddress,
			eConnecting,
			eDisconnecting,
			eListening,
			eAccepting,
			eReading,
			eWriting,
			eGetAvailable,
			eGetHostAddress
		};
		
						CNetworkException( ETask inTask,
									AW_ErrorCode inAWCode
									, const CNetAddress &inAddress
					 				#if DEBUG
					 					, const char inFname[]
					 					, int inLine
					 				#endif
									,OS_ErrorCode inOSCode = kNoOSError )
							: CException( inAWCode
					 			#if DEBUG
					 				, inFname, inLine
					 			#endif
								, inOSCode)
							, mTask( inTask )
				 			, mAddress(inAddress)
				 				{} 
						CNetworkException( const CNetworkException &inExcept )
							: CException(inExcept)
							, mTask( inExcept.mTask )
							, mAddress(inExcept.mAddress)
								{}
						CNetworkException( const CException &inExcept, ETask inTask,
								const CNetAddress &inAddress )
							: CException(inExcept)
							, mTask( inTask )
							, mAddress(inAddress)
								{} 

		ETask			GetTask()
							{ return mTask; }

	private:
		ETask			mTask;
		CNetAddress 	mAddress;
};

#define THROW_NET_( inTask, inErr, inAddr, osErr )\
	throw CNetworkException( (CNetworkException::inTask), (UAWError::inErr), (inAddr) _SOURCE_, (osErr) )
#define THROW_UNKNOWN_NET_( inTask, inAddr )\
	THROW_NET_( inTask, eUnknownExceptionError, inAddr, kNoOSError )
#define THROW_COPY_NET_( inBase, inTask, inAddr )\
	throw CNetworkException( (inBase), (CNetworkException::inTask), (inAddr) )
#define THROW_OS_NET_( osErr, inTask, inAddr )	{ OS_ErrorCode err = (osErr); \
	if(err != kNoOSError) THROW_NET_( inTask, eTranslate, inAddr, err ); }

// This macro makes the default handling of exceptions easier
// Consider a standard Message call, that doesn't want special
// exception handling or remapping.  Do this;
//
// void MessageCall (void) {
//	try {
//		// Code goes here
//	} catch (...) {
//		RETHROW_NET_(whateverTask,address);
//	}
//}
//
// If you want special handlers, put them in before the catch (...) clause.
// Should be done with a function, but compilers aren't ANSI compliant, yet.
//
#define RETHROW_NET_(inTask,inAddr) 		\
{											\
	try {									\
		throw;								\
	} catch (const CNetworkException&) {	\
		throw;								\
	} catch (const CException& e) {			\
		THROW_COPY_NET_(e,inTask,inAddr);	\
	} catch (...) {							\
		THROW_UNKNOWN_NET_(inTask,inAddr);	\
	}										\
}											

HL_End_Namespace_BigRedH
#endif // _H_CNetworkException_
