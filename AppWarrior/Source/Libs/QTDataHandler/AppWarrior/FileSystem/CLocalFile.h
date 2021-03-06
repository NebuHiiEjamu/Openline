/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// A file object to provide basic file access.
// A thin layer over the OS calls.

#ifndef _H_CLocalFile_
#define _H_CLocalFile_

#include "CFSLocalFileRef.h"
#include "CFSException.h"
#include "CMutex.h"

#if PRAGMA_ONCE
	#pragma once
#endif

HL_Begin_Namespace_BigRedH

class CLocalFile
{
	public:
				 				 		CLocalFile( const CFSLocalFileRef &inRef );
											// throws CMemoryException
		virtual 				 		~CLocalFile();
											// throws nothing

			//** Access mode **
		enum EMode {
			eMode_Append,		// create if not exist + adding to the end + write only
			eMode_ReadOnly,		// fail if not exist
			eMode_WriteOnly,	// create if not exist + truncate the file to zero size if exist
			eMode_ReadWrite		// create if not exist
		};
		enum EShareMode {
			eShare_None,		// no sharing
			eShare_Read,		// share only reading
			eShare_Write,		// share only writing
			eShare_ReadWrite	// share both read and write
		};

			//** Opening and closing the file **
		virtual void					Open( EMode inMode,
											EShareMode inShareMode = eShare_None );
											// throws CFSException
		virtual void					Close();
											// throws CFSException
		virtual void					Flush();
											// throws CFSException

			//** Size **
		virtual UInt64					GetSize() const;
											// throws CFSException

			//** Current position **
		enum EFrom {
			eFrom_Start,
			eFrom_Current,
			eFrom_End
		};
		virtual UInt64					GetPosition() const;
											// throws CFSException
		virtual void					SetPosition( EFrom inFrom, SInt64 inDelta );
											// throws CFSException

			//** Reading and writing **
		virtual UInt32					Read( UInt8 *inBuffer, UInt32 inSize );
											// throws CFSException
		virtual UInt32					ReadFrom( UInt8 *inBuffer, UInt32 inSize,
											 UInt64 &ioPosition );
											// throws CFSException
		virtual UInt32					Write( const UInt8 *inBuffer, UInt32 inSize );
											// throws CFSException
		virtual UInt32					WriteFrom( const UInt8 *inBuffer, UInt32 inSize,
											 UInt64 &ioPosition );
											// throws CFSException

	protected:
		std::auto_ptr<CFSLocalFileRef>	mFileRef;
		CMutex							mLock;
		class CLocalFilePS;
		CLocalFilePS&					mPlatform;

}; // class CLocalFile 

HL_End_Namespace_BigRedH
#endif // _H_CLocalFile_
