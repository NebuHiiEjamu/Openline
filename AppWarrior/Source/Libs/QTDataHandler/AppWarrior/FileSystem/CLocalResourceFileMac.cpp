/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "AW.h"
#include "CLocalResourceFileMac.h"
#include "CFSLocalFileRefMac.h"
#include <Files.h>
#include "USyncroHelpers.h"
#include "MoreFilesExtras.h"


HL_Begin_Namespace_BigRedH

class CLocalFile::CLocalFilePS {
	public:
		short				mFileRefNum;
};

// ---------------------------------------------------------------------
//  CLocalResourceFileMac                                       [public]
// ---------------------------------------------------------------------
// constructor

CLocalResourceFileMac::CLocalResourceFileMac( const CFSLocalFileRef &inRef )
	: CLocalFile( inRef )
{
}


// ---------------------------------------------------------------------
//  ~CLocalResourceFileMac	                           [public][virtual]
// ---------------------------------------------------------------------
// destructor

CLocalResourceFileMac::~CLocalResourceFileMac()
{
}


// ---------------------------------------------------------------------
//  Open                                               [public][virtual]
// ---------------------------------------------------------------------
// Open the file

void
CLocalResourceFileMac::Open( EMode inMode, EShareMode inShareMode ) 
{
	ASSERT(mPlatform.mFileRefNum == 0);
	try {
		StMutex lock(mLock);
		const CFSLocalFileRefMac &fileRef =
					*dynamic_cast<const CFSLocalFileRefMac*>(mFileRef.get());
				
		short openMode;
		switch( inMode ){
			case eMode_ReadOnly:
				openMode = dmRd;
				break;
			case eMode_Append:
			case eMode_WriteOnly:
				openMode = dmWr;
				break;
			case eMode_ReadWrite:
				openMode = dmRdWr;
				break;
		}
		switch( inShareMode ){
			case eShare_None:
				openMode |= dmNoneDenyRdWr;
				break;
			case eShare_Read:
				openMode |= dmNoneDenyWr;
				break;
			case eShare_Write:
				openMode |= dmNoneDenyRd;
				break;
		}
		FSSpec theSpec = fileRef.GetFSpec();
		OSErr anErr = FSpOpenRFAware( &theSpec, openMode, &mPlatform.mFileRefNum );
		if( inMode == eMode_ReadOnly ){
			THROW_OS_( anErr );
		} else if( anErr == fnfErr ){
			THROW_OS_( ::FSpCreate( &theSpec, fileRef.GetMacCreator(),
						fileRef.GetMacType(), smSystemScript ) );
			THROW_OS_( FSpOpenRFAware( &theSpec, openMode, &mPlatform.mFileRefNum ) );
		} else {
			THROW_OS_( anErr );
		}
		if( inMode == eMode_Append ){	// go to the end of the file
			THROW_OS_( ::SetFPos( mPlatform.mFileRefNum, fsFromLEOF, 0 ) );
		} else if( inMode == eMode_WriteOnly ){
			THROW_OS_( ::SetEOF( mPlatform.mFileRefNum, 0 ) );
		}	
		// send the apropriate notifications to start the listening devices
	} catch( ... ){
		RETHROW_FS_( eOpen, mFileRef->GetName() );
	}
}


// ---------------------------------------------------------------------
//  GetSize                                            [public][virtual]
// ---------------------------------------------------------------------
// Returns the size of the file

UInt64
CLocalResourceFileMac::GetSize() const
{
	try {
		StMutex lock(mLock);
		short fRef = mPlatform.mFileRefNum;
		SInt32 size;
		if( fRef == 0 ){ // File is not open
			const CFSLocalFileRefMac &fileRef =
						*dynamic_cast<const CFSLocalFileRefMac*>(mFileRef.get());
					
			FSSpec theSpec = fileRef.GetFSpec();
			long dataSize, rsrcSize;
			THROW_OS_( FSpGetFileSize( &theSpec, &dataSize, &rsrcSize ) );
			size = rsrcSize;
		} else { // File is already open
			THROW_OS_( ::GetEOF( fRef, &size ) );
		}
		return size;
	} catch( ... ){
		RETHROW_FS_( eGetSize, mFileRef->GetName() );
	}
	return 0; // to suppress compiler warning
}

HL_End_Namespace_BigRedH
