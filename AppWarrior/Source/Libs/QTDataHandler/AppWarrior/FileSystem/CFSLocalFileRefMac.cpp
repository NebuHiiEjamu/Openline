/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// The Win32 file naming

#include "AW.h"
#include "CFSLocalFileRefMac.h"
#include "CFileStream.h"
#include "FullPath.h"
#include "MoreFilesExtras.h"
//#include "CInternetConfigMac.h"
#include "StString.h"

HL_Begin_Namespace_BigRedH

// ---------------------------------------------------------------------
//  CFSLocalFileRefMac	                                        [public]
// ---------------------------------------------------------------------
// constructor

CFSLocalFileRefMac::CFSLocalFileRefMac(const FSSpec& inFSpec,
							OSType inMacType, OSType inMacCreator )
	: mFSpec(inFSpec)
	, mSpecSetup(true)
	, mMacType(inMacType), mMacCreator(inMacCreator)
	, mSize(0), mSizeLoaded(false)
	, mTypeLoaded(false), mRightsLoaded(false)
{
	// Build filename
	OSErr anErr;
	short len;
	Handle path;
	
	anErr = FSpGetFullPath( &mFSpec,  &len,  &path );
	if( path != nil ){
		::HLock( path );
		mFileName = CString( *path, len );
		::HUnlock( path );
		::DisposeHandle( path );
	}
	SetupTypeCreator();
}

CFSLocalFileRefMac::CFSLocalFileRefMac(const CString& inFileName)
	: mFileName(inFileName)
	, mSpecSetup(false)
	, mMacType('????'), mMacCreator('????')
	, mSize(0), mSizeLoaded(false)
	, mTypeLoaded(false), mRightsLoaded(false)
{
	SetupSpec();
	SetupTypeCreator();
}

CFSLocalFileRefMac::CFSLocalFileRefMac(const CFSLocalFileRefMac& inOther)
	: mFSpec(inOther.mFSpec)
	, mSpecSetup(inOther.mSpecSetup)
	, mMacType(inOther.mMacType), mMacCreator(inOther.mMacCreator)
	, mFileName(inOther.mFileName)
	, mSize(inOther.mSize)
	, mSizeLoaded(inOther.mSizeLoaded)
	, mMimeType(inOther.mMimeType)
	, mTypeLoaded(inOther.mTypeLoaded)
	, mRights(inOther.mRights)
	, mRightsLoaded(inOther.mRightsLoaded)
{}

std::auto_ptr<CFSRef> 
CFSLocalFileRefMac::Clone() const
{
	CFSRef* fsref = nil;
	try {
		fsref = new CFSLocalFileRefMac(*this);
	} catch( ... ){
		RETHROW_FS_( eClone, mFileName );
	}

	return std::auto_ptr<CFSRef> (fsref);
}


// ---------------------------------------------------------------------
//  GetFSpec	                                       [public][virtual]
// ---------------------------------------------------------------------
// get the FSpec of the file

const FSSpec&
CFSLocalFileRefMac::GetFSpec() const 
{ 
	try {
		SetupSpec();
	} catch( ... ){
		RETHROW_FS_( eGetName, mFileName );
	}
	return mFSpec; 
}


// ---------------------------------------------------------------------
//  GetName()	                                       [public][virtual]
// ---------------------------------------------------------------------
// get the name of the file

const CString&
CFSLocalFileRefMac::GetName() const 
{ 
	return mFileName; 
}


// ---------------------------------------------------------------------
//  IsEqual		                                       [public][virtual]
// ---------------------------------------------------------------------
// check if the two file references point to the same actual file

bool
CFSLocalFileRefMac::IsEqual(const CFSRef& inOther) const
{ 
	const CFSLocalFileRefMac* other = dynamic_cast<const CFSLocalFileRefMac*> (&inOther);
	if (other == nil)
		return false;
	if( mSpecSetup && other->mSpecSetup ){ // use the FSpecs
		if( (mFSpec.vRefNum == other->mFSpec.vRefNum) &&
				(mFSpec.parID == other->mFSpec.parID) ){
			// now compare names using current script
			return ::CompareString( mFSpec.name,
						other->mFSpec.name, nil ) == 0;
		} else {
			return false;
		}
	} else {
		return mFileName == other->mFileName;
	}
}	

// ---------------------------------------------------------------------
//  IsLessThan	                                       [public][virtual]
// ---------------------------------------------------------------------
// for ordered pools like std::set or std::map

bool
CFSLocalFileRefMac::IsLessThan(const CFSRef& inOther) const
{
	const CFSLocalFileRefMac* other = dynamic_cast<const CFSLocalFileRefMac*> (&inOther);
	if (other == nil)
		return false;
	return mFileName < other->mFileName;
}

// ---------------------------------------------------------------------
//  LoadInputStream                                    [public][virtual]
// ---------------------------------------------------------------------
// imediately return a stream to the file content

void
CFSLocalFileRefMac::LoadInputStream()
{
	try{
		// check if somebody did request a stream but did not use it
		ASSERT(mInStream.get() == nil);
		mInStream = std::auto_ptr<CInStream> (new CFileInStream(*this));
		Notify(eInput, mInStream.get() != nil);
	} catch( ... ){
		RETHROW_FS_( eLoadInStream, mFileName );
	}
}

// ---------------------------------------------------------------------
//  GetInputStream                                     [public][virtual]
// ---------------------------------------------------------------------
// imediately return a stream to the file content

std::auto_ptr<CInStream>&	
CFSLocalFileRefMac::GetInputStream()
{
	if( mInStream.get() == nil ){
		THROW_UNKNOWN_FS_( eGetInStream, mFileName ); 
	}
	return mInStream;
}

// ---------------------------------------------------------------------
//  LoadOutputStream                                   [public][virtual]
// ---------------------------------------------------------------------
// load a stream to the file content

void
CFSLocalFileRefMac::LoadOutputStream()
{
	try {
		// check if somebody did request a stream but did not use it
		ASSERT(mOutStream.get() == nil);
		mOutStream = std::auto_ptr<COutStream> (new CFileOutStream(*this));
		Notify(eOutput, mOutStream.get() != nil);
	} catch( ... ){
		RETHROW_FS_( eLoadOutStream, mFileName );
	}
}

// ---------------------------------------------------------------------
//  GetOutputStream                                    [public][virtual]
// ---------------------------------------------------------------------
// imediately return a stream to the file content

std::auto_ptr<COutStream>&	
CFSLocalFileRefMac::GetOutputStream()
{
	if( mOutStream.get() == nil ){
		THROW_UNKNOWN_FS_( eGetOutStream, mFileName ); 
	}
	return mOutStream;
}

// ---------------------------------------------------------------------
//  LoadSize		                                   [public][virtual]
// ---------------------------------------------------------------------
// send the size of the file to the listener
// if the file is not there send an error to the listener

void
CFSLocalFileRefMac::LoadSize()
{
	try {
		if (!mSizeLoaded)
		{
			SetupSpec(); // Throws if it doesn't exist
			long dataSize, rsrcSize;
			THROW_OS_( FSpGetFileSize( &mFSpec, &dataSize, &rsrcSize ) );
			mSize = dataSize;
			mSizeLoaded = true;
		}	
		Notify( eSize, mSizeLoaded );
	} catch( ... ){
		RETHROW_FS_( eLoadSize, mFileName );
	}
}

// ---------------------------------------------------------------------
//  GetSize				                               [public][virtual]
// ---------------------------------------------------------------------
// get the previously loaded size

UInt64
CFSLocalFileRefMac::GetSize()
{
	if( !mSizeLoaded ){
		THROW_UNKNOWN_FS_( eGetSize, mFileName );
	}
	return mSize;
}


// ---------------------------------------------------------------------
//  LoadType()		                                   [public][virtual]
// ---------------------------------------------------------------------
// load the type

void
CFSLocalFileRefMac::LoadType()
{
	try {
		if( !mTypeLoaded ){
//			mMimeType = CInternetConfigMac::Instance().GetMIME(
//								mFileName, mMacType, mMacCreator );
			mTypeLoaded = true;
		}	
		Notify(eType,mTypeLoaded);
	} catch( ... ){
		RETHROW_FS_( eGetType, mFileName );
	}
}

// ---------------------------------------------------------------------
//  SetFileType		                                   [public][virtual]
// ---------------------------------------------------------------------
// set the type

void
CFSLocalFileRefMac::SetFileType( const CString& inMime )
{ 
	mMimeType = inMime; 
}


// ---------------------------------------------------------------------
//  GetFileType		                                   [public][virtual]
// ---------------------------------------------------------------------
// get the previously loaded type

CString
CFSLocalFileRefMac::GetFileType()
{
	if( !mTypeLoaded ){
		THROW_UNKNOWN_FS_( eGetType, mFileName );
	}
	return mMimeType; 
}


// ---------------------------------------------------------------------
//  LoadRights			                               [public][virtual]
// ---------------------------------------------------------------------
// load the rights

void
CFSLocalFileRefMac::LoadRights()
{
	try {
		if( !mRightsLoaded ){
			SetupSpec(); // Throws if it doesn't exist
			FInfo fndrInfo;
			THROW_OS_( ::FSpGetFInfo( &mFSpec, &fndrInfo ) );
			mRights.SetVisible( !(fndrInfo.fdFlags & kIsInvisible) );
			mRights.SetRead( true );
			mRights.SetWrite( true );
			mRightsLoaded = true;
		}
		Notify(eRights, mRightsLoaded);
	} catch( ... ){
		RETHROW_FS_( eLoadRights, mFileName );
	}
}


// ---------------------------------------------------------------------
//  SaveRights			                               [public][virtual]
// ---------------------------------------------------------------------
// save the rights (apply them to the file)

/*
void
CFSLocalFileRefMac::SaveRights()
{
	bool saved = false;
	try
	{
		FInfo fndrInfo;
		OSErr anErr = ::FSpGetFInfo( &mFSpec, &fndrInfo );
		THROW_OS_FS_( anErr, eSaveRights, mFileName );
		anErr = ::FSpSetFInfo( &mFSpec, &fndrInfo );
		THROW_OS_FS_( anErr, eLoadRights, mFileName );
		saved = true;
	}
	catch (CException& err)
	{}
	Notify(eWRights, saved);
}
*/


// ---------------------------------------------------------------------
//  GetRights			                               [public][virtual]
// ---------------------------------------------------------------------
// get the previously loaded rights

CFileRights&
CFSLocalFileRefMac::GetRights()
{
	if( !mRightsLoaded ){
		THROW_UNKNOWN_FS_( eGetRights, mFileName );
	}
	return mRights;
}


// ---------------------------------------------------------------------
//  Delete()		                                   [public][virtual]
// ---------------------------------------------------------------------
// delete the file and notify the listener

void
CFSLocalFileRefMac::Delete()
{
	try {
		SetupSpec(); // Throws if it doesn't exist
		THROW_OS_( ::FSpDelete( &mFSpec ) );
		Notify( eDeleted, true );
	} catch( ... ){
		RETHROW_FS_( eDelete, mFileName );
	}
}


// ---------------------------------------------------------------------
//  SetupTypeCreator                                         [protected]
// ---------------------------------------------------------------------
// Setup the Mac Type and Creator if not filled in

void
CFSLocalFileRefMac::SetupTypeCreator()
{
	try {
		if( mMacType == '????' ){
			SetupSpec(); // Throws if it doesn't exist
			FInfo fndrInfo;
			if( ::FSpGetFInfo( &mFSpec, &fndrInfo ) == noErr ){
				mMacType = fndrInfo.fdType;
				mMacCreator = fndrInfo.fdCreator;
			} else { // use IC
//				CInternetConfigMac::Instance().GetTypeCreator( mFileName,
//								mMacType, mMacCreator );
			}
		}
	} catch( ... ){
	}
}


// ---------------------------------------------------------------------
//  SetupSpec                                                [protected]
// ---------------------------------------------------------------------
// Sets up the FSpec from the file name if required

void
CFSLocalFileRefMac::SetupSpec( bool inMustExist ) const
{
	if( !mSpecSetup ){
		OSErr anErr;
		CString name = mFileName;
		if( name.length() < 256 ){ // Use MacOS call if name short
									// enough for a Pascal string
			StPStyleString s( name );
			anErr = ::FSMakeFSSpec( 0, 0, s, &mFSpec );
			if( (anErr == noErr) || ((anErr == fnfErr) && !inMustExist) ){
					// Found it or found its parent if is doesn't need to exist
				mSpecSetup = true;
			} else {
				THROW_OS_( anErr );
			}
		} else { // Use MoreFiles
			StCStyleString s( name );
			anErr = FSpLocationFromFullPath( name.length(), s, &mFSpec );
			if( anErr == noErr ){
				mSpecSetup = true;
			} else {
				THROW_OS_( anErr );
			}
		}
	}
}

HL_End_Namespace_BigRedH
