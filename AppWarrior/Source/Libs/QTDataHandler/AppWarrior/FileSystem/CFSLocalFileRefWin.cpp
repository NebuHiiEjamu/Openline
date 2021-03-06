/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// The Win32 file naming

#include "AW.h"
#include "CFSLocalFileRefWin.h"
#include "CFileStream.h"
#include "StString.h"

HL_Begin_Namespace_BigRedH
/*
// ---------------------------------------------------------------------
//  Upcase														[static]
// ---------------------------------------------------------------------
//?? ... big question mark about this function ...
//		 it can only upcase a-z characters

static CString
Upcase (const CString& inString)
{
	CString str = inString;

	CString::iterator i = str.begin();
	CString::iterator e = str.end();
	for (;i != e; ++i)
	{
		wchar_t c = *i;
		// convert a-z to A-Z , 
		//?? what do we do with the other characters
		// Win32 (NT, 2000) allows you to do UNICODE comparisons natively
		if ((c >= L'a') && (c <= L'z'))
			*i = CString::value_type(c - L'a' + L'A');
	}

	return str;
}
*/

// ---------------------------------------------------------------------
//  CFSLocalFileRefWin	                                        	[public]
// ---------------------------------------------------------------------
// constructor

CFSLocalFileRefWin::CFSLocalFileRefWin(const CString& inFileName)
: mSize(0)
, mSizeLoaded(false), mTypeLoaded(false), mRightsLoaded(false)
, mFileName(inFileName)
{}

// ---------------------------------------------------------------------
//  CFSLocalFileRefWin	                                        	[public]
// ---------------------------------------------------------------------
// constructor

CFSLocalFileRefWin::CFSLocalFileRefWin( const CString& inFileName
							, UInt32 inAttr) 
: mSize(0)
, mSizeLoaded(false), mTypeLoaded(false), mRightsLoaded(false)
, mFileName(inFileName)
{}

// ---------------------------------------------------------------------
//  CFSLocalFileRefWin	                                        	[public]
// ---------------------------------------------------------------------
// constructor

CFSLocalFileRefWin::CFSLocalFileRefWin(const CFSLocalFileRefWin& inOther)
: mSize(inOther.mSize)
, mSizeLoaded(inOther.mSizeLoaded)
, mTypeLoaded(inOther.mTypeLoaded)
, mRightsLoaded(inOther.mRightsLoaded)
, mFileName(inOther.mFileName)
, mMimeType(inOther.mMimeType)
, mRights(inOther.mRights)
{}

// ---------------------------------------------------------------------
//  Clone	                                           [public][virtual]
// ---------------------------------------------------------------------
// clone this

std::auto_ptr<CFSRef> 
CFSLocalFileRefWin::Clone() const
{
	CFSRef* fsref = nil;
	try
	{
		fsref = new CFSLocalFileRefWin(*this);
	}
	catch (...) { RETHROW_FS_(eClone,mFileName); }

	return std::auto_ptr<CFSRef> (fsref);
}

// ---------------------------------------------------------------------
//  GetName()	                                       [public][virtual]
// ---------------------------------------------------------------------
// get the name of the file

const CString&
CFSLocalFileRefWin::GetName() const
{ 
	return mFileName; 
}

// ---------------------------------------------------------------------
//  GetFileExtension                                   		 [protected]
// ---------------------------------------------------------------------
// get the extension part of the file name

CString
CFSLocalFileRefWin::GetFileExtension() const
{
	// get the extension
	UInt32 pos = mFileName.find_last_of(L'.');
	if (mFileName.find(L'/',pos) != CString::npos)
		pos = CString::npos;
	if (mFileName.find(L'\\',pos) != CString::npos)
		pos = CString::npos;
	if (pos == CString::npos)
		return CString();
	else	
		return mFileName.substr(pos);	
}

// ---------------------------------------------------------------------
//  IsEqual		                                       [public][virtual]
// ---------------------------------------------------------------------
// check if the two file references point to the same actual file

bool
CFSLocalFileRefWin::IsEqual(const CFSRef& inOther) const
{ 
	const CFSLocalFileRefWin* other = dynamic_cast<const CFSLocalFileRefWin*> (&inOther);
	if (other == nil)
		return false;
	return mFileName == other->mFileName;
}	

// ---------------------------------------------------------------------
//  IsLessThan	                                       [public][virtual]
// ---------------------------------------------------------------------
// for ordered pools like std::set or std::map

bool
CFSLocalFileRefWin::IsLessThan(const CFSRef& inOther) const
{
	const CFSLocalFileRefWin* other = dynamic_cast<const CFSLocalFileRefWin*> (&inOther);
	if (other == nil)
		return false;
	return mFileName < other->mFileName;
}

// ---------------------------------------------------------------------
//  LoadInputStream                                    [public][virtual]
// ---------------------------------------------------------------------
// imediately return a stream to the file content

void
CFSLocalFileRefWin::LoadInputStream()
{
	try
	{
		// check if somebody did request a stream but did not use it
		if(mInStream.get() != nil)
			throw 1;
		mInStream = std::auto_ptr<CInStream> (new CFileInStream(*this));
		Notify(eInput, mInStream.get() != nil);
	}
	catch (...) { RETHROW_FS_(eLoadInStream,mFileName); }
}

// ---------------------------------------------------------------------
//  GetInputStream                                     [public][virtual]
// ---------------------------------------------------------------------
// imediately return a stream to the file content

std::auto_ptr<CInStream>&	
CFSLocalFileRefWin::GetInputStream()
{
	if (mInStream.get() == nil)
	{ THROW_UNKNOWN_FS_(eGetInStream,mFileName); }
	return mInStream;
}

// ---------------------------------------------------------------------
//  LoadOutputStream                                   [public][virtual]
// ---------------------------------------------------------------------
// load a stream to the file content

void
CFSLocalFileRefWin::LoadOutputStream()
{
	try
	{
		// check if somebody did request a stream but did not use it
		if (mOutStream.get() != nil)
			throw 1;
		mOutStream = std::auto_ptr<COutStream> (new CFileOutStream(*this));
		Notify(eOutput, mOutStream.get() != nil);
	}	
	catch (...) { RETHROW_FS_(eLoadOutStream,mFileName); }
}

// ---------------------------------------------------------------------
//  GetOutputStream                                    [public][virtual]
// ---------------------------------------------------------------------
// imediately return a stream to the file content

std::auto_ptr<COutStream>&	
CFSLocalFileRefWin::GetOutputStream()
{
	if (mOutStream.get() == nil)
	{
		THROW_UNKNOWN_FS_(eGetOutStream,mFileName); 
	}
	return mOutStream;
}

// ---------------------------------------------------------------------
//  LoadSize		                                   [public][virtual]
// ---------------------------------------------------------------------
// send the size of the file to the listener
// if the file is not there send an error to the listener

void
CFSLocalFileRefWin::LoadSize()
{
	try
	{
		if (!mSizeLoaded)
		{
			StCStyleString fileNameOS(mFileName);
			HANDLE handle = ::CreateFile( fileNameOS
							, 0 //GENERIC_READ
							, FILE_SHARE_READ | FILE_SHARE_WRITE
							, NULL
							, OPEN_EXISTING
							, 0
							, NULL );
			//?? should throw a more specific file exception here
			if (handle == INVALID_HANDLE_VALUE)
				THROW_OS_(::GetLastError());
		
			UInt64 size = 0;
			0[(UInt32*)&size]  = ::GetFileSize(handle, &1[(UInt32*)&size]);
			OS_ErrorCode error = ::GetLastError();
			if ((0[(UInt32*)&size] == UInt32(-1)) && (NO_ERROR != error))
				THROW_OS_(error);
			
			::CloseHandle(handle);
			mSize = size;
			mSizeLoaded = true;
		}	
		Notify(eSize,mSizeLoaded);
	}
	catch (...) { RETHROW_FS_(eLoadSize,mFileName); }
}

// ---------------------------------------------------------------------
//  GetSize				                               [public][virtual]
// ---------------------------------------------------------------------
// get the previously loaded size

UInt64
CFSLocalFileRefWin::GetSize()
{
	if (!mSizeLoaded)
	{
		THROW_UNKNOWN_FS_(eGetSize,mFileName); 
	}
	return mSize;
}

// ---------------------------------------------------------------------
//  GetMimeFromRegistry										 [protected]
// ---------------------------------------------------------------------

bool
CFSLocalFileRefWin::GetMimeFromRegistry()
{
	bool ok = true;

	const CString theExtension = GetFileExtension();
	const CString::value_type* extension = theExtension.c_str();
	if (theExtension.empty())
		extension = L"*";
	
	// registry keys are case insensitive so we don't really care 
	// to go upcase or lowcase here
	StCStyleString extensionOS(extension);
	StCStyleString valNameOS(L"Content Type");
	UInt32 	   mimeOS_Size = 1024;
	
	{	StCopybackCStyleString mimeOS(mMimeType, mimeOS_Size);

		HKEY hKey = nil;
		ok &= (ERROR_SUCCESS == ::RegOpenKey(HKEY_CLASSES_ROOT,extensionOS,&hKey));
		ok &= (ERROR_SUCCESS == ::RegQueryValueExA(hKey,valNameOS,NULL,NULL,(unsigned char*)(char*)mimeOS,&mimeOS_Size));
		ok &= (ERROR_SUCCESS == ::RegCloseKey(hKey));
	}
	if (!ok) mMimeType = CString("");
	return ok;	
}


// ---------------------------------------------------------------------
//  LoadType()		                                   [public][virtual]
// ---------------------------------------------------------------------
// load the type

void
CFSLocalFileRefWin::LoadType()
{
	try
	{
		if (! mTypeLoaded)
		{
			if (GetMimeFromRegistry())
				mTypeLoaded = true;
/*
			else
			{	
				CString::value_type* mime = nil;
				OS_ErrorCode oserr = (OS_ErrorCode)::FindMimeFromData( NULL
													 , mFileName.c_str()
													 , NULL, 0, NULL, 0
													 , &mime, 0);
				THROW_OS_(oserr);
				mMimeType = mime;
				mTypeLoaded = true;
			}	
*/
		}	
		Notify(eType,mTypeLoaded);
	}
	catch (...) { RETHROW_FS_(eLoadType,mFileName); }
}

// ---------------------------------------------------------------------
//  SetFileType		                                   [public][virtual]
// ---------------------------------------------------------------------
// set the type

void
CFSLocalFileRefWin::SetFileType(const CString& inMime)
{ 
	try
	{
		mMimeType = inMime; 
	}	
	catch (...) { RETHROW_FS_(eSetType,mFileName); }
}

// ---------------------------------------------------------------------
//  GetFileType		                                   [public][virtual]
// ---------------------------------------------------------------------
// get the previously loaded type

CString
CFSLocalFileRefWin::GetFileType()
{
	if (!mTypeLoaded)
	{ THROW_UNKNOWN_FS_(eGetType,mFileName); }
	return mMimeType.c_str(); 
}

// ---------------------------------------------------------------------
//  LoadRights			                               [public][virtual]
// ---------------------------------------------------------------------
// load the rights

void
CFSLocalFileRefWin::LoadRights()
{
	try
	{
		if (!mRightsLoaded)
		{
			StCStyleString fileNameOS(mFileName);
			DWORD attr = ::GetFileAttributes(fileNameOS);
			if (DWORD(-1) == attr) 
				THROW_OS_(::GetLastError());
			InitRightsFrom(attr);
		}
		Notify(eRights, mRightsLoaded);
	}
	catch (...) { RETHROW_FS_(eLoadRights,mFileName); }
}

// ---------------------------------------------------------------------
//  InitRightsFrom		                               		 [protected]
// ---------------------------------------------------------------------
// convert Win32 file attributes to CFileRights

void
CFSLocalFileRefWin::InitRightsFrom(UInt32 inAttr)
{
	mRightsLoaded = (	(DWORD(-1) != inAttr) 
					&&	(0 == (FILE_ATTRIBUTE_DIRECTORY & inAttr)) );
	//FILE_ATTRIBUTE_COMPRESSED
	//FILE_ATTRIBUTE_ENCRYPTED
	mRights.SetWrite(0 == (FILE_ATTRIBUTE_READONLY & inAttr));
	mRights.SetVisible(0 == (FILE_ATTRIBUTE_HIDDEN & inAttr));
	mRights.SetRead(true);
}

// ---------------------------------------------------------------------
//  GetRights			                               [public][virtual]
// ---------------------------------------------------------------------
// get the previously loaded rights

CFileRights&
CFSLocalFileRefWin::GetRights()
{
	if(!mRightsLoaded)
	{ THROW_UNKNOWN_FS_(eGetRights,mFileName); }
	return mRights;
}

// ---------------------------------------------------------------------
//  Delete()		                                   [public][virtual]
// ---------------------------------------------------------------------
// delete the file and notify the listener

void
CFSLocalFileRefWin::Delete()
{
	try
	{
		StCStyleString fileNameOS(mFileName);
		// don't allow readonly files to be deleted
		DWORD attr = ::GetFileAttributes(fileNameOS);
		if (DWORD(-1) == attr) 
			THROW_OS_(::GetLastError());
		// the system does not protect read-only files but we do
		if (0 != (FILE_ATTRIBUTE_READONLY & attr))
			THROW_OS_(ERROR_WRITE_PROTECT);
		// time to remove the file
		if (0 == ::DeleteFile(fileNameOS))
			THROW_OS_(::GetLastError());
		Notify(eDeleted, true);
	}
	catch (...) { RETHROW_FS_(eDelete,mFileName); }
}


HL_End_Namespace_BigRedH
