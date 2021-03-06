/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// A file object to provide basic file access to the resource fork on the Mac.
// A thin layer over the OS calls.

#ifndef _H_CLocalResourceFileMac_
#define _H_CLocalResourceFileMac_

#include "CLocalFile.h"

#if PRAGMA_ONCE
	#pragma once
#endif

HL_Begin_Namespace_BigRedH

class CLocalResourceFileMac : public CLocalFile
{
	public:
				 				 		CLocalResourceFileMac( const CFSLocalFileRef &inRef );
											// throws CMemoryException
		virtual 				 		~CLocalResourceFileMac();
											// throws nothing

			//** Opening the file **
		virtual void					Open( EMode inMode,
											EShareMode inShareMode = eShare_None );
											// throws CFSException

			//** Size **
		virtual UInt64					GetSize() const;
											// throws CFSException

}; // class CLocalResourceFileMac 

HL_End_Namespace_BigRedH
#endif // _H_CLocalResourceFileMac_
