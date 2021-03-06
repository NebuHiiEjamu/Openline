// =====================================================================
//  CFSException.h                       (C) Hotline Communications 2000
// =====================================================================
// File problems , let the user try again if possible

#ifndef _H_CFSException_
#define _H_CFSException_

#include "CException.h"

#if PRAGMA_ONCE
	#pragma once
#endif				   

HL_Begin_Namespace_BigRedH

class CFSException : public CException
{
	public:
		enum 			ETask 
							{ 
								eClone, eCreateStorage, eReleaseStorage
							  , eGetName
							  , eGetContainerRef, eGetChildRef
							  , eLoadRights, eGetRights
							  , eLoadContent, eGetContent
							  , eCreate, eDelete
							  ,	eLoadInStream, eGetInStream
							  ,	eLoadOutStream, eGetOutStream
							  , eLoadSize, eGetSize
							  , eLoadType, eGetType, eSetType
							  , eLoadObject, eGetObject
							  , eOpen, eClose
							  , eFlush, eGetPosition
							  , eSetPosition, eRead
							  , eWrite
							  , eGetFSRef
							  , eGetTempFolder
							  , eChooseAFile
							  , eChooseMultipleFiles
							  , eChooseAFolder
							  , eChooseMultipleFolders
							  , eChooseAFileOrFolder
							  , eChooseMultipleFilesOrFolders
							};

						CFSException( ETask inTask
									, AW_ErrorCode inAWCode
									, const CString &inFSName
					 				_SOURCEARG_
									, OS_ErrorCode inOSCode = kNoOSError )
							// throws ???
							: CException( inAWCode _SOURCECOPY_	, inOSCode)
							, mTask( inTask )
				 			, mFSName(inFSName)
				 			{} 

						CFSException( const CException& inOld
									, ETask inTask
									, const CString &inFSName )
							// throws ???
						 	: CException( inOld )		
							, mTask( inTask )
				 			, mFSName( inFSName )
				 			{} 
						
						CFSException( const CFSException &inExcept )
							// throws ???
							: CException(inExcept)
							, mTask( inExcept.mTask )
							, mFSName(inExcept.mFSName)
							{} 

		ETask			GetTask()
							// throws nothing
							{ return mTask; }

	private:
		ETask			mTask;
		CString 		mFSName;
}; // class CFSException

#define THROW_FS_( inTask, inErr, fName, osErr  ) throw\
	CFSException( CFSException::inTask, UAWError::inErr, (fName) _SOURCE_, (osErr) )
#define THROW_COPY_FS_( inBase, inTask, fName ) throw\
	CFSException( (inBase), CFSException::inTask, (fName) )
#define THROW_OS_FS_( osErr, inTask, fName  )	{ OS_ErrorCode err = (osErr); \
	if(err != kNoOSError) THROW_FS_(inTask,eTranslate,fName,err); }
#define THROW_UNKNOWN_FS_( inTask, fName  ) \
	THROW_FS_(inTask, eUnknownExceptionError, fName, kNoOSError)

// This macro makes the default handling of exceptions easier
// Consider a standard Message call, that doesn't want special
// exception handling or remapping.  Do this;
//
// void MessageCall (void) {
//	try {
//		// Code goes here
//	} catch (...) {
//		RETHROW_FS_(whateverTask,filename);
//	}
//}
//
// If you want special handlers, put them in before the catch (...) clause.
// Should be done with a function, but compilers aren't ANSI compliant, yet.
//
#define RETHROW_FS_(inTask,fName) 			\
{											\
	try {									\
		throw;								\
	} catch (const CFSException&) {			\
		throw;								\
	} catch (const CException& e) {			\
		THROW_COPY_FS_(e,inTask,fName);		\
	} catch (...) {							\
		THROW_UNKNOWN_FS_(inTask,fName);	\
	}										\
}											

HL_End_Namespace_BigRedH
#endif // _H_CFileException_
