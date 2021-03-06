/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// The Win32 file naming

#include "AW.h"
#include "CFSLocalFolderRefWin.h"
#include "CFSLocalFileRefWin.h"
#include "StString.h"

HL_Begin_Namespace_BigRedH

// ---------------------------------------------------------------------
//  CFSLocalFolderRefWin	                                        	[public]
// ---------------------------------------------------------------------
// constructor

CFSLocalFolderRefWin::CFSLocalFolderRefWin(const CString& inFileName)
: mFolderName(inFileName)
, mRightsLoaded(false)
{}

// ---------------------------------------------------------------------
//  CFSLocalFolderRefWin	                                        	[public]
// ---------------------------------------------------------------------
// constructor

CFSLocalFolderRefWin::CFSLocalFolderRefWin(const CString& inFileName, UInt32 inAttr)
: mFolderName(inFileName)
, mRightsLoaded(false)
{
	InitRightsFrom(inAttr);
}

// ---------------------------------------------------------------------
//  CFSLocalFolderRefWin	                                        	[public]
// ---------------------------------------------------------------------
// constructor

CFSLocalFolderRefWin::CFSLocalFolderRefWin(const CFSLocalFolderRefWin& inOther)
: mFolderName(inOther.mFolderName)
, mRights(inOther.mRights)
, mContentLoaded(false) //?? let's not duplicate the content on cloning
, mRightsLoaded(inOther.mRightsLoaded)
{}

// ---------------------------------------------------------------------
//  Clone			                                   [public][virtual]
// ---------------------------------------------------------------------
// clone this

std::auto_ptr<CFSRef> 
CFSLocalFolderRefWin::Clone() const
{
	CFSRef* fsref = nil;
	try
	{
		fsref = new CFSLocalFolderRefWin(*this);
	}
	catch (...) { RETHROW_FS_(eClone,mFolderName); }

	return std::auto_ptr<CFSRef> (fsref);
	
}

// ---------------------------------------------------------------------
//  GetName()	                                       [public][virtual]
// ---------------------------------------------------------------------
// get the name of the file

const CString&
CFSLocalFolderRefWin::GetName() const
{ 
	return mFolderName; 
}

// ---------------------------------------------------------------------
//  IsEqual		                                       [public][virtual]
// ---------------------------------------------------------------------
// check if the two file references point to the same actual file

bool
CFSLocalFolderRefWin::IsEqual(const CFSRef& inOther) const
{ 
	const CFSLocalFolderRefWin* other = dynamic_cast<const CFSLocalFolderRefWin*> (&inOther);
	if (other == nil)
		return false;
	return mFolderName == other->mFolderName;
}	

// ---------------------------------------------------------------------
//  IsLessThan	                                       [public][virtual]
// ---------------------------------------------------------------------
// for ordered pools like std::set or std::map
bool
CFSLocalFolderRefWin::IsLessThan(const CFSRef& inOther) const
{
	const CFSLocalFolderRefWin* other = dynamic_cast<const CFSLocalFolderRefWin*> 
										(&inOther);
	if (other == nil)
		return false;
	return mFolderName < other->mFolderName;
}

// ---------------------------------------------------------------------
//  LoadRights			                               [public][virtual]
// ---------------------------------------------------------------------
// load the rights

void
CFSLocalFolderRefWin::LoadRights()
{
	try
	{
		if (!mRightsLoaded)
		{
			StCStyleString fileNameOS(mFolderName);
			DWORD attr = ::GetFileAttributes(fileNameOS);
			InitRightsFrom(attr);
		}
		Notify(eRights, mRightsLoaded);
	}
	catch (...) { RETHROW_FS_(eLoadRights,mFolderName); }
}

// ---------------------------------------------------------------------
//  InitRightsFrom		                               		 [protected]
// ---------------------------------------------------------------------
// convert Win32 file attributes to CFileRights

void
CFSLocalFolderRefWin::InitRightsFrom(UInt32 inAttr)
{
	if (inAttr == DWORD(-1))
		THROW_OS_(::GetLastError());
	mRightsLoaded = (0 != (FILE_ATTRIBUTE_DIRECTORY & inAttr) );
	if (! mRightsLoaded)
		THROW_OS_(ERROR_DIRECTORY);
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
CFSLocalFolderRefWin::GetRights()
{
	if (!mRightsLoaded)
	{ THROW_UNKNOWN_FS_(eGetRights,mFolderName); }
	return mRights;
}

// ---------------------------------------------------------------------
//  StFindFileWin
// ---------------------------------------------------------------------
// helper class used by LoadContent()

class StFindFileWin
{
	public:
				StFindFileWin(const CString& inFolderName);
					// throws ???
				~StFindFileWin();
					// throws nothing
		void 	Next();
					// throws ???
		bool	Found()
					// throws nothing
					{ return !mDone; }
		WIN32_FIND_DATA&	GetData()
					{ return mData; }				
	private:
		HANDLE 				mHandle;	
		WIN32_FIND_DATA 	mData;
		bool 				mDone;
}; // class StFindFileWin

// ---------------------------------------------------------------------
//  StFindFileWin	                                   			[public]
// ---------------------------------------------------------------------
// constructor

StFindFileWin::StFindFileWin(const CString& inFolderName)
: mHandle(INVALID_HANDLE_VALUE)
, mDone(true)
{
	StCStyleString fileNameOS(inFolderName);
	mHandle = ::FindFirstFile(fileNameOS,&mData);
	bool success = (mHandle != INVALID_HANDLE_VALUE);
	mDone  = (!success && (ERROR_NO_MORE_FILES == ::GetLastError()));
	if (!success && !mDone)
		THROW_OS_(GetLastError());
}

// ---------------------------------------------------------------------
//  ~StFindFileWin	                                   			[public]
// ---------------------------------------------------------------------
// destructor

StFindFileWin::~StFindFileWin()
{
	try
	{
		if (mHandle != INVALID_HANDLE_VALUE)
			if(0 != ::FindClose(mHandle))
				ASSERT(false);
	}
	catch (...) 
	{}	
}

// ---------------------------------------------------------------------
//  Next			                                   			[public]
// ---------------------------------------------------------------------
// get next file in the folder

void 
StFindFileWin::Next()
{
	bool success = (0 != ::FindNextFile(mHandle,&mData));
	mDone  = (!success && (ERROR_NO_MORE_FILES == ::GetLastError()));
	if (!success && !mDone)
		THROW_OS_(GetLastError());
}	
		


// ---------------------------------------------------------------------
//  LoadContent		                                   [public][virtual]
// ---------------------------------------------------------------------
// retrieve the folder's content

void
CFSLocalFolderRefWin::LoadContent()
{
	try
	{
		if (! mContentLoaded)
		{
			CString pattern = mFolderName + L"\\*.*";
			for(StFindFileWin files(pattern); files.Found(); files.Next())
			{
				WIN32_FIND_DATA& data =	files.GetData();
				CFSRef* theItem = nil;
				CString fileName(data.cFileName);
				// skip . and .. filenames
				if ((fileName.length() == 1) && (fileName[0] == L'.'))
				   continue;
				if ((fileName.length() == 2) && (fileName[0] == L'.') && (fileName[1] == L'.'))
				   continue;
				CString fullName = mFolderName;
				fullName += CString(L"\\");
				fullName += fileName;
				// remember folders and normal files differently   
				if (0 == (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					theItem = new CFSLocalFileRefWin(fullName.c_str(),data.dwFileAttributes);
				else
					theItem = new CFSLocalFolderRefWin(fullName.c_str(),data.dwFileAttributes);
				mContent.push_back(theItem);
			}
			mContentLoaded = true;
		}	
		Notify(eLoaded, mContentLoaded);
	}
	catch (...) { RETHROW_FS_(eLoadContent,mFolderName); }
}

// ---------------------------------------------------------------------
//  GetContent		                                   [public][virtual]
// ---------------------------------------------------------------------
// get the items in a folder

const CFSLocalFolderRefWin::Content& 
CFSLocalFolderRefWin::GetContent()
{
	if (!mContentLoaded)
	{ THROW_UNKNOWN_FS_(eGetContent,mFolderName); }
	return mContent;
}

// ---------------------------------------------------------------------
//  Create			                                   [public][virtual]
// ---------------------------------------------------------------------
// create an empty folder

void
CFSLocalFolderRefWin::Create()
{
	try
	{
		StCStyleString fileNameOS(mFolderName);
		if(0 == ::CreateDirectory(fileNameOS,NULL))
			THROW_OS_(::GetLastError());
		Notify(eCreated, true);
	}
	catch (...) { RETHROW_FS_(eCreate,mFolderName); }
}

// ---------------------------------------------------------------------
//  Delete()		                                   [public][virtual]
// ---------------------------------------------------------------------
// delete the file and notify the listener

void
CFSLocalFolderRefWin::Delete()
{
	try
	{
		StCStyleString fileNameOS(mFolderName);

		// don't allow readonly folders to be deleted
		DWORD attr = ::GetFileAttributes(fileNameOS);
		if (DWORD(-1) == attr) 
			THROW_OS_(::GetLastError());
		// the system does not protect read-only files but we do
		if (0 != (FILE_ATTRIBUTE_READONLY & attr))
			THROW_OS_(ERROR_WRITE_PROTECT);
		// time to remove the folder
  		if (0 == ::RemoveDirectory(fileNameOS))
			THROW_OS_(::GetLastError());

		Notify(eDeleted, true);
	}
	catch (...) { RETHROW_FS_(eDelete,mFolderName); }
}

HL_End_Namespace_BigRedH
