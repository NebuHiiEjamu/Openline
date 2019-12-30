/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "MoreTypes.h"
#include "UDateTime.h"

/*
 * Types
 */

class TFSRefObj;

/*
 * Constants
 */

enum {
	fsItemType_NonExistantParent	= 0,		// item and enclosing folder don't exist
	fsItemType_NonExistant			= 1,		// item doesn't exist, but enclosing folder does
	fsItemType_File					= 2,
	fsItemType_Folder				= 3,
	fsItemType_FileAlias			= 4,
	fsItemType_FolderAlias			= 5,
	fsItemType_UnattachedAlias		= 6
};


// New() options
enum {
	fsOption_NilIfNonExistantParent	= 0x0001,	// return nil if enclosing folder can't be found (item doesn't exist either ofcourse)
	fsOption_NilIfNonExistant		= 0x0002,	// return nil if item doesn't exist OR enclosing folder can't be found
	fsOption_NilIfExists			= 0x0004,
	fsOption_NilIfNotFile			= 0x0008,
	fsOption_NilIfNotFolder			= 0x0010,
	fsOption_FailIfNonExistant		= 0x0020,	// fail if item doesn't exist OR enclosing folder can't be found
	fsOption_FailIfExists			= 0x0040,
	fsOption_FailIfNotFile			= 0x0080,
	fsOption_FailIfNotFolder		= 0x0100,
	fsOption_ResolveAliases			= 0x0200,
	
	fsOption_RequireExistingFile	= fsOption_FailIfNonExistant + fsOption_FailIfNotFile + fsOption_ResolveAliases,
	fsOption_RequireExistingFolder	= fsOption_FailIfNonExistant + fsOption_FailIfNotFolder + fsOption_ResolveAliases,
	fsOption_PreferExistingFile		= fsOption_NilIfNonExistant + fsOption_NilIfNotFile + fsOption_ResolveAliases,
	fsOption_PreferExistingFolder	= fsOption_NilIfNonExistant + fsOption_NilIfNotFolder + fsOption_ResolveAliases
};

// CreateFileAndOpen() options
enum {
	kCreateNewFile		= 1,	// create and open a new file, fail if file already exists
	kAlwaysCreateFile	= 2,	// create and open a new file, overwrite any existing file
	kOpenExistingFile	= 3,	// open the file, fail if file doesn't exist
	kAlwaysOpenFile		= 4		// open the file, create if doesn't already exist
};

// GetListing() options
enum {
	kDontResolveAliases	= 0x0001
};

// special starting folders for New()
static TFSRefObj* const kRootFolderHL = (TFSRefObj*)max_Uint32;
static TFSRefObj* const kProgramFolder = (TFSRefObj*)(max_Uint32-1);
static TFSRefObj* const kTempFolder = (TFSRefObj*)(max_Uint32-2);
static TFSRefObj* const kPrefsFolder = (TFSRefObj*)(max_Uint32-3);
static TFSRefObj* const kDesktopFolder = (TFSRefObj*)(max_Uint32-4);

/*
 * Structures
 */

#pragma options align=packed
struct SFSListItem {
	Uint32 typeCode, creatorCode, size, flags, rsvd;
	SDateTimeStamp createdDate, modifiedDate;
	Uint8 name[];
};
#pragma options align=reset

#pragma options align=packed
struct SFlattenRef {
    Uint32 sig;     // 'FSrf'
    Uint16 vers;    // 1
    Uint32 OS;      // 'MACH' or 'WIND'

    Uint32 dataSize;
    Uint8 data[];
};
#pragma options align=reset


/*
 * File System Manager
 */

class UFileSys
{
	public:
		// references to file system objects
		static TFSRefObj* New(TFSRefObj* inStartFolder, 
							  const Uint8 *inPath, const Uint8 *inName, 
							  Uint32 inOptions = 0, Uint16 *outType = nil);
		static TFSRefObj* New(TFSRefObj* inStartFolder, 
							  const void *inPath, Uint32 inPathSize, const Uint8 *inName, 
							  Uint32 inOptions = 0, Uint16 *outType = nil);
		static TFSRefObj* NewTemp(TFSRefObj* inRef = nil);
		
		static TFSRefObj* Clone(TFSRefObj* inRef);
		
		static void Dispose(TFSRefObj* inRef);
		static void SetRef(TFSRefObj* inDst, TFSRefObj* inSrc);
		
		#if MACINTOSH
		static SInt16 OpenResourceFork(TFSRefObj* inStartFolder, 
							  const UInt8 *inPath, const Uint8 *inName,
							  SInt8 rw = 0);
		#endif
		
		// creating objects
		static void CreateFile(TFSRefObj* inRef, Uint32 inTypeCode, Uint32 inCreatorCode);
		static void CreateFileAndOpen(TFSRefObj* inRef, Uint32 inTypeCode, Uint32 inCreatorCode, Uint32 inOptions, Uint32 inPerm = perm_ReadWrite);
		static void CreateFolder(TFSRefObj* inRef);
		static void CreateAlias(TFSRefObj* inRef, TFSRefObj* inOriginal);
		static bool Exists(TFSRefObj* inRef, bool *outIsFolder = nil);
		
		// deleting objects
		static void DeleteFile(TFSRefObj* inRef);
		static void DeleteFolder(TFSRefObj* inRef);
		static void DeleteFolderContents(TFSRefObj* inRef);
		static void MoveToTrash(TFSRefObj* inRef);
		
		// renaming objects
		static void SetName(TFSRefObj* ioRef, const Uint8 *inName, Uint32 inEncoding = 0);
		static void SetRefName(TFSRefObj* ioRef, const Uint8 *inName, Uint32 inEncoding = 0);
		static void GetName(TFSRefObj* inRef, Uint8 *outName, Uint32 *outEncoding = nil);
		static Uint32 GetName(TFSRefObj* inRef, void *outData, Uint32 inMaxSize, Uint32 *outEncoding = nil);
		static bool EnsureUniqueName(TFSRefObj* inRef);
		static Uint32 GetPath(TFSRefObj* inRef, void *outPath, Uint32 inMaxSize, Uint32 *outEncoding = nil);
		static Uint32 GetFolder(TFSRefObj* inRef, void *outFolder, Uint32 inMaxSize, Uint32 *outEncoding = nil);
		static TFSRefObj* GetFolder(TFSRefObj* inRef);
		static void GetParentName(TFSRefObj* inRef, Uint8 *outName, Uint32 *outEncoding = nil);
		static Uint32 GetParentName(TFSRefObj* inRef, void *outData, Uint32 inMaxSize, Uint32 *outEncoding = nil);
		static Uint16 GetNumberName(TFSRefObj* inFolder, Uint16 inNum, Uint8 *ioName, Uint32 inEncoding = 0);
		static Uint32 ValidateFileName(void *ioName, Uint32 inSize, Uint32 inEncoding = 0);
		static Uint32 ValidateFilePath(void *ioPath, Uint32 inPathSize);

		// relocating objects
		static void Move(TFSRefObj* ioSource, TFSRefObj* inDestFolder);
		static void MoveAndRename(TFSRefObj* ioSource, TFSRefObj* inDest, const Uint8 *inNewName, Uint32 inEncoding = 0);
		static void SwapData(TFSRefObj* inRefA, TFSRefObj* inRefB);
		
		// reading and writing file data
		static void Open(TFSRefObj* inRef, Uint32 inPerm = perm_ReadWrite);
		static void Close(TFSRefObj* inRef);
		static Uint32 Read(TFSRefObj* inRef, Uint32 inOffset, void *outData, Uint32 inMaxSize, Uint32 inOptions = 0);
		static Uint32 Write(TFSRefObj* inRef, Uint32 inOffset, const void *inData, Uint32 inDataSize, Uint32 inOptions = 0);
		static Uint32 GetSize(TFSRefObj* inRef, Uint32 inOptions = 0);
		static void SetSize(TFSRefObj* inRef, Uint32 inSize);
		static void Flush(TFSRefObj* inRef = nil);
		static THdl ReadToHdl(TFSRefObj* inRef, Uint32 inOffset, Uint32& ioSize, Uint32 inOptions = 0);
		static bool Copy(TFSRefObj* inSource, TFSRefObj* ioDest, Uint32 inDestTypeCode, Uint32 inDestCreatorCode);
		
		// object properties
		static Uint16 GetType(TFSRefObj* inRef);
		static void SetComment(TFSRefObj* inRef, const void *inText, Uint32 inTextSize, Uint32 inEncoding = 0);
		static Uint32 GetComment(TFSRefObj* inRef, void *outText, Uint32 inMaxSize, Uint32 *outEncoding = nil);
		static void SetTypeAndCreatorCode(TFSRefObj* inRef, Uint32 inTypeCode, Uint32 inCreatorCode);
		static void GetTypeAndCreatorCode(TFSRefObj* inRef, Uint32& outTypeCode, Uint32& outCreatorCode);
		static Uint32 GetTypeCode(TFSRefObj* inRef);
		static Uint32 GetCreatorCode(TFSRefObj* inRef);
		static void GetDateStamp(TFSRefObj* inRef, SDateTimeStamp *outModified, SDateTimeStamp *outCreated = nil);
		static bool IsHidden(TFSRefObj* inRef);
		static bool Equals(TFSRefObj* inRef, TFSRefObj* inCompare);
		
		// user interaction
		static TFSRefObj* UserSelectFile(TFSRefObj* inInitianFolder = nil, const Uint32 *inTypeList = nil, Uint32 inOptions = 0);
		static TFSRefObj* UserSelectFolder(TFSRefObj* inInitianFolder = nil, Uint32 inOptions = 0);
		static TFSRefObj* UserSaveFile(TFSRefObj* inInitianFolder = nil, const Uint8 *inDefaultName = nil, const Uint8 *inPrompt = nil, Uint32 inNameEncoding = 0, Uint32 inPromptEncoding = 0, Uint32 inOptions = 0);
		
		// listing folders
		static THdl GetListing(TFSRefObj* inFolder, Uint32 inOptions = 0);
		static bool GetListNext(THdl inListData, Uint32& ioOffset, Uint8 *outName, Uint32 *outTypeCode = nil, Uint32 *outCreatorCode = nil, Uint32 *outSize = nil, SDateTimeStamp *outCreatedDate = nil, SDateTimeStamp *outModifiedDate = nil, Uint32 *outFlags = nil);
	
		// paths
		static Uint32 GetPathTargetName(const void *inPath, Uint32 inPathSize, void *outName, Uint32 inMaxSize, Uint32 *outEncoding = nil, Uint8 **outPtr = nil);
		static void *GetParentPathData(const void *inPath, Uint32 inPathSize, Uint32& outPathSize);
		static Uint32 AppendToPath(void *ioPath, Uint32 inPathSize, const Uint8 *inName, Uint32 inNameSize, Uint32 inNameEncoding = 0);	// ioPath must be inPathSize + inNameSize + 3
		static void *MakePathData(const void *inPath, Uint32 inPathSize, const Uint8 *inName, Uint32& outPathSize);
		static void *MakePathData(const void *inPath1, Uint32 inPathSize1, const void *inPath2, Uint32 inPathSize2, Uint32& outPathSize);
		static Uint32 GetPathSize(const void *inPath, Uint16 inOverrideCount = max_Uint16);
		static Uint32 GetApplicationPath(void *outAppPath, Uint32 inMaxPathSize);
		static Uint32 GetApplicationURL(UInt8 *url, Uint32 urlZ);
		static Uint32 GetTempPath(void *outTempPath, Uint32 inMaxPathSize);

		// path convertion
		static Uint32 ConvertPathDataToUrl(const void *inPath, Uint32 inPathSize, void *outData, Uint32 inMaxSize);
		static Uint32 ConvertUrlToPathData(const void *inPath, Uint32 inPathSize, void *outData, Uint32 inMaxSize);

		// misc
		static bool ResolveAlias(TFSRefObj* ioRef, Uint16 *outOrigType = nil);
		static bool IsDiskLocked(TFSRefObj* inRef);
		static Uint64 GetDiskFreeSpace(TFSRefObj* inRef);
		
		// flatten/unflatten ref
		static SFlattenRef *FlattenRef(TFSRefObj* inRef);
		static TFSRefObj* UnflattenRef(SFlattenRef *inFlattenRef);
		
		// flattening
		static void StartFlatten(TFSRefObj* inRef);
		static void ResumeFlatten(TFSRefObj* inRef, const void *inResumeData, Uint32 inDataSize);
		static void StopFlatten(TFSRefObj* inRef);
		static Uint32 GetFlattenSize(TFSRefObj* inRef);
		static Uint32 ProcessFlatten(TFSRefObj* inRef, void *outData, Uint32 inMaxSize);
		static bool IsFlattening(TFSRefObj* inRef);
		
		// unflattening
		static void StartUnflatten(TFSRefObj* inRef);
		static THdl ResumeUnflatten(TFSRefObj* inRef);
		static void StopUnflatten(TFSRefObj* inRef);
		static bool ProcessUnflatten(TFSRefObj* inRef, const void *inData, Uint32 inDataSize);
		static bool IsUnflattening(TFSRefObj* inRef);
};
typedef UFileSys UFS;

/*
 * Stack TFSRefObj*
 */

class StFileSysRef
{
	public:
		StFileSysRef(TFSRefObj* inStartFolder, const Uint8 *inPath, const Uint8 *inName, Uint32 inOptions = 0, Uint16 *outType = nil)					{	mRef = UFileSys::New(inStartFolder, inPath, inName, inOptions, outType);				}
		StFileSysRef(TFSRefObj* inStartFolder, const void *inPath, Uint32 inPathSize, const Uint8 *inName, Uint32 inOptions = 0, Uint16 *outType = nil)	{	mRef = UFileSys::New(inStartFolder, inPath, inPathSize, inName, inOptions, outType);	}
		~StFileSysRef()						{	UFileSys::Dispose(mRef);	}
		operator TFSRefObj*()					{	return mRef;				}
		TFSRefObj *operator->() const	{	return mRef;				}
		bool IsValid()						{	return mRef != nil;			}
		bool IsInvalid()					{	return mRef == nil;			}

	private:
		TFSRefObj* mRef;
};
typedef StFileSysRef StFSRef;

class StFileOpener
{
	public:
		StFileOpener(TFSRefObj* inRef, Uint16 inPerm = perm_ReadWrite) : mRef(inRef)	
		{	UFileSys::Open(mRef, inPerm);	}
		~StFileOpener()																
		{	UFileSys::Close(mRef);			}
		operator TFSRefObj*()															
		{	return mRef;					}
		TFSRefObj *operator->() const											
		{	return mRef;					}

	private:
		TFSRefObj* mRef;
};

/*
 * File System Error Codes
 */

enum {
	errorType_FileSys					= 4,
	fsError_Unknown						= 100,
	fsError_NotEnoughSpace				= fsError_Unknown + 1,
	fsError_NoSuchItem					= fsError_Unknown + 2,
	fsError_NoSuchFile					= fsError_Unknown + 3,
	fsError_NoSuchFolder				= fsError_Unknown + 4,
	fsError_NoSuchDisk					= fsError_Unknown + 5,
	fsError_DiskDamaged					= fsError_Unknown + 6,
	fsError_DiskSoftwareDamaged			= fsError_Unknown + 7,
	fsError_DiskHardwareDamaged			= fsError_Unknown + 8,
	fsError_DiskFormatUnknown			= fsError_Unknown + 9,
	fsError_OperationNotSupported		= fsError_Unknown + 10,
	fsError_ItemLocked					= fsError_Unknown + 11,
	fsError_DiskLocked					= fsError_Unknown + 12,
	fsError_FileLocked					= fsError_Unknown + 13,
	fsError_FolderLocked				= fsError_Unknown + 14,
	fsError_ItemAlreadyExists			= fsError_Unknown + 15,
	fsError_FileInUse					= fsError_Unknown + 16,
	fsError_AccessDenied				= fsError_Unknown + 17,
	fsError_ChangeDenied				= fsError_Unknown + 18,
	fsError_BadName						= fsError_Unknown + 19,
	fsError_EndOfFile					= fsError_Unknown + 20,
	fsError_DiskOffline					= fsError_Unknown + 21,
	fsError_BadMove						= fsError_Unknown + 22,
	fsError_DiskDisappeared				= fsError_Unknown + 23,
	fsError_FileExpected				= fsError_Unknown + 24,
	fsError_FolderExpected				= fsError_Unknown + 25,
	fsError_SameItem					= fsError_Unknown + 26,
	fsError_UserNotAuth					= fsError_Unknown + 27,
	fsError_CannotResolveAlias			= fsError_Unknown + 28,
	fsError_DifferentDisks				= fsError_Unknown + 29,
	fsError_PathToLong					= fsError_Unknown + 30,
	fsError_ChildPathToLong				= fsError_Unknown + 31
};

/*
 * UFileSys Object Interface
 */

class TFSRefObj
{
	public:
		TFSRefObj* Clone()																				{	return UFS::Clone(this);										}
		void SetRef(TFSRefObj* inSrc)																	{	UFS::SetRef(this, inSrc);										}

		void CreateFile(Uint32 inTypeCode, Uint32 inCreatorCode)									{	UFS::CreateFile(this, inTypeCode, inCreatorCode);				}
		void CreateFileAndOpen(Uint32 inTypeCode, Uint32 inCreatorCode, Uint32 inOptions, Uint32 inPerm = perm_ReadWrite)	{	UFS::CreateFileAndOpen(this, inTypeCode, inCreatorCode, inOptions, inPerm);	}
		void CreateFolder()																			{	UFS::CreateFolder(this);										}
		void CreateAlias(TFSRefObj* inOriginal)															{	UFS::CreateAlias(this, inOriginal);								}
		bool Exists(bool *outIsFolder = nil)														{	return UFS::Exists(this, outIsFolder);							}
		
		void DeleteFile()																			{	UFS::DeleteFile(this);											}
		void DeleteFolder()																			{	UFS::DeleteFolder(this);										}
		void DeleteFolderContents()																	{	UFS::DeleteFolderContents(this);								}
		void MoveToTrash()																			{	UFS::MoveToTrash(this);											}
		
		void SetName(const Uint8 *inName, Uint32 inEncoding = 0)									{	UFS::SetName(this, inName, inEncoding);							}
		void SetRefName(const Uint8 *inName, Uint32 inEncoding = 0)									{	UFS::SetRefName(this, inName, inEncoding);						}
		void GetName(Uint8 *outName, Uint32 *outEncoding = nil)										{	UFS::GetName(this, outName, outEncoding);						}
		Uint32 GetName(void *outData, Uint32 inMaxSize, Uint32 *outEncoding = nil)					{	return UFS::GetName(this, outData, inMaxSize, outEncoding);		}
		bool EnsureUniqueName()																		{	return UFS::EnsureUniqueName(this);								}
		Uint32 GetPath(void *outPath, Uint32 inMaxSize, Uint32 *outEncoding = nil)					{	return UFS::GetPath(this, outPath, inMaxSize, outEncoding);		}
		Uint32 GetFolder(void *outFolder, Uint32 inMaxSize, Uint32 *outEncoding = nil)				{	return UFS::GetFolder(this, outFolder, inMaxSize, outEncoding);	}
		TFSRefObj* GetFolder()																		
		{	return UFS::GetFolder(this); }
		void GetParentName(Uint8 *outName, Uint32 *outEncoding = nil)								{	UFS::GetParentName(this, outName, outEncoding);					}
		Uint32 GetParentName(void *outData, Uint32 inMaxSize, Uint32 *outEncoding = nil)			{	return UFS::GetParentName(this, outData, inMaxSize, outEncoding);	}

		void Move(TFSRefObj* inDestFolder)																{	UFS::Move(this, inDestFolder);									}
		void MoveAndRename(TFSRefObj* inDest, const Uint8 *inNewName, Uint32 inEncoding = 0)			{	UFS::MoveAndRename(this, inDest, inNewName, inEncoding);		}
		void SwapData(TFSRefObj* inRef)																	{	UFS::SwapData(this, inRef);										}
		
		void Open(Uint32 inPerm = perm_ReadWrite)													{	UFS::Open(this, inPerm);										}
		void Close()																				{	UFS::Close(this);												}

		Uint32 Read(Uint32 inOffset, void *outData, Uint32 inMaxSize, Uint32 inOptions = 0)
		{ return UFS::Read(this, inOffset, outData, inMaxSize, inOptions); }
		
		Uint32 Write(Uint32 inOffset, const void *inData, Uint32 inDataSize, Uint32 inOptions = 0)	
		{ return UFS::Write(this, inOffset, inData, inDataSize, inOptions); }
		
		Uint32 GetSize(Uint32 inOptions = 0)														
		{ return UFS::GetSize(this, inOptions); }
		
		void SetSize(Uint32 inSize)												
		{ UFS::SetSize(this, inSize); }
		
		void Flush()															
		{ UFS::Flush(this); }
		
		THdl ReadToHdl(Uint32 inOffset, Uint32& ioSize, Uint32 inOptions = 0)						{	return UFS::ReadToHdl(this, inOffset, ioSize, inOptions);		}
		bool Copy(TFSRefObj* ioDest, Uint32 inDestTypeCode, Uint32 inDestCreatorCode)					{	return UFS::Copy(this, ioDest, inDestTypeCode, inDestCreatorCode);	}

		Uint16 GetType()																			{	return UFS::GetType(this);										}
		void SetComment(const void *inText, Uint32 inTextSize, Uint32 inEncoding = 0)				{	UFS::SetComment(this, inText, inTextSize, inEncoding);			}
		Uint32 GetComment(void *outText, Uint32 inMaxSize, Uint32 *outEncoding = nil)				{	return UFS::GetComment(this, outText, inMaxSize, outEncoding);	}
		void SetTypeAndCreatorCode(Uint32 inTypeCode, Uint32 inCreatorCode)							{	UFS::SetTypeAndCreatorCode(this, inTypeCode, inCreatorCode);	}
		void GetTypeAndCreatorCode(Uint32& outTypeCode, Uint32& outCreatorCode)						{	UFS::GetTypeAndCreatorCode(this, outTypeCode, outCreatorCode);	}
		Uint32 GetTypeCode()																		{	return UFS::GetTypeCode(this);									}
		Uint32 GetCreatorCode()																		{	return UFS::GetCreatorCode(this);								}
		void GetDateStamp(SDateTimeStamp *outModified, SDateTimeStamp *outCreated = nil)			{	UFS::GetDateStamp(this, outModified, outCreated);				}
		bool IsHidden()																				{	return UFS::IsHidden(this);										}
		bool Equals(TFSRefObj* inCompare)																{	return UFS::Equals(this, inCompare);							}

		THdl GetListing(Uint32 inOptions = 0)														{	return UFS::GetListing(this, inOptions);						}

		bool ResolveAlias(Uint16 *outOrigType = nil)												{	return UFS::ResolveAlias(this, outOrigType);					}
		bool IsDiskLocked()																			{	return UFS::IsDiskLocked(this);									}
		Uint64 GetDiskFreeSpace()																	{	return UFS::GetDiskFreeSpace(this);								}
		
		void StartFlatten()																			{	UFS::StartFlatten(this);										}
		void ResumeFlatten(const void *inResumeData, Uint32 inDataSize)								{	UFS::ResumeFlatten(this, inResumeData, inDataSize);				}
		void StopFlatten()																			{	UFS::StopFlatten(this);											}
		Uint32 GetFlattenSize()																		{	return UFS::GetFlattenSize(this);								}
		Uint32 ProcessFlatten(void *outData, Uint32 inMaxSize)										{	return UFS::ProcessFlatten(this, outData, inMaxSize);			}
		bool IsFlattening()																			{	return UFS::IsFlattening(this);									}

		SFlattenRef *FlattenRef()																	{	return UFS::FlattenRef(this);									}

		void StartUnflatten()																		{	UFS::StartUnflatten(this);										}
		THdl ResumeUnflatten()																		{	return UFS::ResumeUnflatten(this);								}
		void StopUnflatten()																		{	UFS::StopUnflatten(this);										}
		bool ProcessUnflatten(const void *inData, Uint32 inDataSize)								{	return UFS::ProcessUnflatten(this, inData, inDataSize);			}
		bool IsUnflattening()																		{	return UFS::IsUnflattening(this);								}

		void operator delete(void *p)																{	UFS::Dispose((TFSRefObj*)p);										}
	protected:
		TFSRefObj() {}				// force creation via UFileSys
};


/*
 * CTempFile
 */
 
class CTempFile
{
	public:
		CTempFile();
		virtual ~CTempFile();

		virtual bool SetTempFile(const Uint8 *inFileName, Uint32 inTypeCode, Uint32 inCreatorCode, bool inDefaultExtension = true);
		void SetFileInfo(const Uint8 *inFileInfo);

		TFSRefObj* GetFile();
		TFSRefObj* GetCloneFile();
		bool SaveFileAs(TFSRefObj* inInitianFolder = nil);
		bool GetFileName(Uint8 *outFileName, Uint32 inMaxSize);
		bool GetTempName(Uint8 *outTempFileName, Uint32 inMaxSize);
		Uint32 GetTypeCode();
		Uint32 GetTempTypeCode();
		Uint32 GetCreatorCode();
		Uint32 GetTempCreatorCode();
		Uint32 GetChecksum();
		Uint32 GetFileSize();
		Uint32 GetFileInfo(Uint8 *outFileInfo, Uint32 inMaxSize);

		virtual bool WriteFileData(const void *inBuffer, Uint32 inBufferSize);
		virtual bool ReadFileData(void *outBuffer, Uint32 inBufferSize);
		virtual bool ReadFileData(Uint32 inOffset, void *outBuffer, Uint32 inBufferSize);
		virtual THdl ReadFileData(Uint32& outTypeCode);

		virtual bool CompareFileName(const Uint8 *inFileName, Uint8 *outTempFileName = nil);
		virtual bool CompareTempFileName(const Uint8 *inTempFileName, Uint8 *outFileName = nil);
		virtual bool IsValidFile();
		virtual void DeleteFile();
		virtual void MoveToTrash();

	protected:
		static Uint32 mTempCount;

		TFSRefObj* mFile;
		Uint8 mFileName[256];
		Uint8 mTempFileName[20];
		Uint8 mFileInfo[64];
		Uint32 mTypeCode;
		Uint32 mCreatorCode;
		Uint32 mChecksum;
};


/*
 * CEncodedTempFile
 */
 
class CEncodedTempFile : public CTempFile
{
	public:
		CEncodedTempFile();
		virtual ~CEncodedTempFile();
				
		virtual bool SetTempFile(const Uint8 *inFileName, Uint32 inTypeCode, Uint32 inCreatorCode, bool inDefault = true);

		virtual bool WriteFileData(const void *inBuffer, Uint32 inBufferSize);
		virtual THdl ReadFileData(Uint32& outTypeCode);

		virtual bool IsValidFile();
		virtual void DeleteFile();
				
	protected:
		bool mWasEncoded;
		
		Uint32 mFileSize;
		SDateTimeStamp mCreatedDate;

		Int32 mFileCrcStart;
		Uint32 mFileCrc;
		
		Int32 mFileAdlerStart;
		Uint32 mFileAdler;
		
		Uint32 mEncodingOffset[25];
		
		bool EncodeFileData(const void *inBuffer, Uint32 inBufferSize);
		void EncodeBuffer(const void *inBuffer, Uint32 inBufferSize);
		void DecodeBuffer(const void *inBuffer, Uint32 inBufferSize);
};


/*
 * CPurgedTempFile
 */

class CPurgedTempFile : public CTempFile
{
	public:
		CPurgedTempFile(Uint32 inMaxFileSize = 1048576);	// 1M
		virtual ~CPurgedTempFile();
	
		Uint32 GetWriteSize();
		bool CanWriteSize(Uint32 inBufferSize);
		virtual bool WriteFileData(const void *inBuffer, Uint32 inBufferSize);

		Uint32 GetReadSize();
		bool CanReadSize(Uint32 inBufferSize);
		virtual bool ReadFileData(void *outBuffer, Uint32 inBufferSize);
								
	protected:
		Uint32 mMaxFileSize;
		Uint32 mWriteOffset;
		Uint32 mReadOffset;
};


/*
 * CGeneralFile
 */
 
class CGeneralFile : public CTempFile
{
	public:
		CGeneralFile(bool inPurge = true);
		virtual ~CGeneralFile();

		virtual void SetGeneralFile(TFSRefObj* inFolder, Uint8 *inFileName, Uint32 inTypeCode, Uint32 inCreatorCode, bool inCreateNew = true);
				
		virtual bool SaveFileAs(TFSRefObj* inInitianFolder = nil);
		virtual void DeleteFile();
		virtual void MoveToTrash();
				
	protected:
		bool mPurge;
};
