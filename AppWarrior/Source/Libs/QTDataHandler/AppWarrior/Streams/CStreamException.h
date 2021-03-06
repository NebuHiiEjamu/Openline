// =====================================================================
//  CStreamException.h                   (C) Hotline Communications 1999
// =====================================================================
// File problems , let the user try again if possible

#ifndef _H_CStreamException_
#define _H_CStreamException_

#include "CException.h"

#if PRAGMA_ONCE
	#pragma once
#endif				   

HL_Begin_Namespace_BigRedH

class CStreamException : public CException
{
	public:
		enum ETask 
		{ 
			  eIsSource
			, eGet, ePut
			, eRead, eWrite
			, eOpen, eClose
			, eCanWrite, eCanRead
			, eQuery, eTimeout
		};
		
						CStreamException( ETask inTask
										, AW_ErrorCode inAWCode
							 			  _SOURCEARG_
										, OS_ErrorCode inOSCode = kNoOSError )
							// throws ???
							: CException( inAWCode _SOURCECOPY_, inOSCode)
							, mTask( inTask )
			 				{} 
						CStreamException( const CException &inExcept
										, ETask inTask )
							// throws ???
							: CException(inExcept)
							, mTask( inTask )
							{} 
						CStreamException( const CStreamException &inExcept )
							// throws ???
							: CException(inExcept)
							, mTask( inExcept.mTask )
							{} 

		ETask			GetTask()
							// throws nothing
							{ return mTask; }

	private:
		ETask			mTask;
};

#define THROW_STREAM_( inTask, inErr, osErr )\
	throw CStreamException( (CStreamException::inTask), (UAWError::inErr) _SOURCE_, (osErr) )
#define THROW_UNKNOWN_STREAM_( inTask )\
	THROW_STREAM_( inTask, eUnknownExceptionError, kNoOSError )
#define THROW_COPY_STREAM_( inBase, inTask )\
	throw CStreamException( inBase, CStreamException::inTask )
#define THROW_OS_STREAM_( osErr, inTask )	{ OS_ErrorCode err = (osErr); \
	if(err != kNoOSError) THROW_STREAM_( inTask, eTranslate, err ); }

// This macro makes the default handling of exceptions easier
// Consider a standard Message call, that doesn't want special
// exception handling or remapping.  Do this;
//
// void MessageCall (void) {
//	try {
//		// Code goes here
//	} catch (...) {
//		RETHROW_STREAM_(whateverTask);
//	}
//}
//
// If you want special handlers, put them in before the catch (...) clause.
// Should be done with a function, but compilers aren't ANSI compliant, yet.
//
#define RETHROW_STREAM_(inTask) 			\
{											\
	try {									\
		throw;								\
	} catch (const CStreamException&) {		\
		throw;								\
	} catch (const CException& e) {			\
		THROW_COPY_STREAM_(e,inTask);		\
	} catch (...) {							\
		THROW_UNKNOWN_STREAM_(inTask);		\
	}										\
}											

HL_End_Namespace_BigRedH
#endif // _H_CStreamException_
