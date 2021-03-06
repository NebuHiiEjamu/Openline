/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// A Windows-specific wrapper for Win32 file-system calls.

#include "AW.h"
#include "CLocalFile.h"
#include "CFSLocalFileRefWin.h"
#include "StString.h"
#include "USyncroHelpers.h"

HL_Begin_Namespace_BigRedH

//	A windows-specific class contains all necessary info
class CLocalFile::CLocalFilePS
{
	public:
		CLocalFilePS() : mFileHandle( INVALID_HANDLE_VALUE )	{}
		~CLocalFilePS()		{ mFileHandle = INVALID_HANDLE_VALUE; }

		HANDLE	mFileHandle;		//	the file handle
};


// ---------------------------------------------------------------------
//  CLocalFile                                               	[public]
// ---------------------------------------------------------------------
// constructor

CLocalFile::CLocalFile(const CFSLocalFileRef& inRef)
: mPlatform( *new CLocalFilePS )
{
	mFileRef.reset( (CFSLocalFileRef*)inRef.Clone().release() );
}

// ---------------------------------------------------------------------
//  ~CLocalFile	                                             	[public]
// ---------------------------------------------------------------------
// constructor

CLocalFile::~CLocalFile()
{
	try {
		if (INVALID_HANDLE_VALUE != mPlatform.mFileHandle)
			this->Close();
	}
	catch (...)	{
		delete &mPlatform;
		throw;
	}

	delete &mPlatform;
}

// ---------------------------------------------------------------------
//  Open	                                                    [public]
// ---------------------------------------------------------------------
// open the file

void
CLocalFile::Open(EMode inMode, EShareMode inShareMode)
{
	try {
		StMutex lock(mLock);

		if (INVALID_HANDLE_VALUE == mPlatform.mFileHandle) {
			StCStyleString		fileNameOS(mFileRef->GetName());
			UInt32 access = (eMode_ReadOnly==inMode) ? GENERIC_READ :
							((eMode_ReadWrite==inMode) ? GENERIC_READ|GENERIC_WRITE : GENERIC_WRITE );
			UInt32 shareMode = (eShare_ReadWrite==inShareMode) ? FILE_SHARE_READ|FILE_SHARE_WRITE :
							   ((eShare_Read==inShareMode) ? FILE_SHARE_READ :
							    ((eShare_Write==inShareMode) ? FILE_SHARE_WRITE : 0));
			UInt32 createOpt = (eMode_ReadOnly==inMode) ? OPEN_EXISTING :
							   ((eMode_WriteOnly==inMode) ? CREATE_NEW : OPEN_ALWAYS );

			mPlatform.mFileHandle = ::CreateFile(	fileNameOS,				// lpFileName
													access,					// dwDesiredAccess
													shareMode,				// share mode
													null,					// lpSecurityAttributes
													createOpt,				// dwCreationDisposition
													FILE_ATTRIBUTE_NORMAL,	// dwFlagsAndAttributes,
													null );					// hTemplateFile

			//	For WriteOnly, open and truncate the file if already exists
			if ((eMode_WriteOnly==inMode) &&
				(INVALID_HANDLE_VALUE == mPlatform.mFileHandle) ) {
				mPlatform.mFileHandle = ::CreateFile(	fileNameOS,				// lpFileName
														access,					// dwDesiredAccess
														shareMode,				// share mode
														null,					// lpSecurityAttributes
														TRUNCATE_EXISTING,		// dwCreationDisposition
														FILE_ATTRIBUTE_NORMAL,	// dwFlagsAndAttributes,
														null );					// hTemplateFile
			}

			if (INVALID_HANDLE_VALUE == mPlatform.mFileHandle) {
				THROW_OS_(::GetLastError());
			}
			else if (eMode_Append==inMode)	// move the file pointer to the end of the file
				this->SetPosition(eFrom_End, 0);
		}
		else {
			THROW_FS_(eOpen, eFileAlreadyOpen, mFileRef->GetName(), kNoOSError );
		}
	}
	catch (...) { RETHROW_FS_(eOpen,mFileRef->GetName()); }
}

// ---------------------------------------------------------------------
//  Close	                                                    [public]
// ---------------------------------------------------------------------
// close the file

void
CLocalFile::Close()
{
	try {
		StMutex lock(mLock);

		if (::CloseHandle(mPlatform.mFileHandle))
			mPlatform.mFileHandle = INVALID_HANDLE_VALUE;
		else {
			THROW_OS_(::GetLastError());
		}
	}
	catch (...) { RETHROW_FS_(eClose,mFileRef->GetName()); }
}

// ---------------------------------------------------------------------
//  Flush	                                                    [public]
// ---------------------------------------------------------------------
// flush the data in the buffer to the file

void
CLocalFile::Flush()
{
	try {
		StMutex lock(mLock);

		if (!::FlushFileBuffers(mPlatform.mFileHandle)) {
			THROW_OS_(::GetLastError());
		}
	}
	catch (...) { RETHROW_FS_(eFlush,mFileRef->GetName()); }
}

// ---------------------------------------------------------------------
//  GetSize	                                                    [public]
// ---------------------------------------------------------------------
// return the file size

UInt64
CLocalFile::GetSize() const
{
	ULARGE_INTEGER	size = {0};

	ASSERT(sizeof(SInt64)==sizeof(ULARGE_INTEGER));
	try {
		StMutex lock(mLock);

		if (INVALID_HANDLE_VALUE != mPlatform.mFileHandle) {	//	already open
			size.LowPart = ::GetFileSize(mPlatform.mFileHandle, &size.HighPart);

			if ((-1 == size.LowPart) && (NO_ERROR != ::GetLastError())) {	// failed
				THROW_OS_(::GetLastError());
			}
		}
		else {	// don't try to open the file in order to just get the size of it
			StCStyleString		fileNameOS(mFileRef->GetName());
			WIN32_FIND_DATA	fileData = {0};
			HANDLE			searchHandle = ::FindFirstFile(fileNameOS, &fileData);

			if (INVALID_HANDLE_VALUE == searchHandle) {
				THROW_OS_(::GetLastError());
			}
			else {
				BOOL retCode = ::FindClose(searchHandle);

				size.HighPart	= fileData.nFileSizeHigh;
				size.LowPart	= fileData.nFileSizeLow;
			}
		}
	}
	catch (...) { RETHROW_FS_(eGetSize,mFileRef->GetName()); }

	return *((UInt64*)&size);
}

// ---------------------------------------------------------------------
//  GetPosition	                                                [public]
// ---------------------------------------------------------------------
// return the current file pointer's position

UInt64
CLocalFile::GetPosition() const
{
	LARGE_INTEGER	pos = {0};

	ASSERT(sizeof(SInt64)==sizeof(LARGE_INTEGER));
	try {
		StMutex lock(mLock);

		pos.LowPart = ::SetFilePointer(	mPlatform.mFileHandle,
										pos.LowPart,
										&pos.HighPart,
										FILE_CURRENT);

		// Should have used INVALID_SET_FILE_POINTER to check for error instead of -1;
		// however, it's not defined anywhere
		if ((-1 == pos.LowPart) && (NO_ERROR != ::GetLastError())) {	// failed
			THROW_OS_(::GetLastError());
		}
	}
	catch (...) { RETHROW_FS_(eGetPosition,mFileRef->GetName()); }

	return *((UInt64*)&pos);
}

// ---------------------------------------------------------------------
//  SetPosition	                                                [public]
// ---------------------------------------------------------------------
// set the current file pointer's position

void
CLocalFile::SetPosition(EFrom inFrom, SInt64 inDelta)
{
	ASSERT(sizeof(SInt64)==sizeof(LARGE_INTEGER));
	try {
		StMutex lock(mLock);
		DWORD	method = (eFrom_Start==inFrom) ? FILE_BEGIN :
						 ((eFrom_End==inFrom) ? FILE_END : FILE_CURRENT);
		LARGE_INTEGER&	pos = (LARGE_INTEGER&)inDelta;

		pos.LowPart = ::SetFilePointer(	mPlatform.mFileHandle,
										pos.LowPart,
										&pos.HighPart,
										method);

		// Should have used INVALID_SET_FILE_POINTER to check for error instead of -1;
		// however, it's not defined anywhere
		if ((-1 == pos.LowPart) && (NO_ERROR != ::GetLastError())) {	// failed
			THROW_OS_(::GetLastError());
		}
	}
	catch (...) { RETHROW_FS_(eSetPosition,mFileRef->GetName()); }
}

// ---------------------------------------------------------------------
//  Read	                                                    [public]
// ---------------------------------------------------------------------
// read the specified number of bytes into the supplied buffer

UInt32
CLocalFile::Read(UInt8 *outBuffer, UInt32 inSize)
{
	UInt32	bytesRead = 0;

	try {
		StMutex lock(mLock);

		if (!::ReadFile(mPlatform.mFileHandle, outBuffer, inSize, &bytesRead, null )) {
			THROW_OS_(::GetLastError());
		}
	}
	catch (...) { RETHROW_FS_(eRead,mFileRef->GetName()); }

	return bytesRead;
}

// ---------------------------------------------------------------------
//  ReadFrom                                                    [public]
// ---------------------------------------------------------------------
// Read up to inSize bytes of data into the buffer from the ioPosition
// bytes from the start of the file. Return the actual number of bytes read.

UInt32
CLocalFile::ReadFrom(UInt8 *inBuffer, UInt32 inSize, UInt64 &ioPosition)
{
	UInt32	bytesRead = 0;

	try {
		StMutex lock(mLock);

		this->SetPosition(eFrom_Start, ioPosition);
		bytesRead = this->Read(inBuffer, inSize);
		ioPosition = this->GetPosition();
	}
	catch (...) { RETHROW_FS_(eRead,mFileRef->GetName()); }

	return bytesRead;
}

// ---------------------------------------------------------------------
//  Write	                                                    [public]
// ---------------------------------------------------------------------
// write the specified number of bytes into the file at the current file
// pointer's position

UInt32
CLocalFile::Write(const UInt8 *inBuffer, UInt32 inSize)
{
	UInt32	bytesWritten = 0;

	try {
		StMutex lock(mLock);

		if (!::WriteFile(mPlatform.mFileHandle, inBuffer, inSize, &bytesWritten, null )) {
			THROW_OS_(::GetLastError());
		}
	}
	catch (...) { RETHROW_FS_(eWrite,mFileRef->GetName()); }

	return bytesWritten;
}

// ---------------------------------------------------------------------
//  WriteFrom                                                   [public]
// ---------------------------------------------------------------------
// write up to inSize bytes of data from the buffer from the ioPosition
// bytes from the start of the file. Return the acutal number of bytes
// written.

UInt32
CLocalFile::WriteFrom(const UInt8 *inBuffer, UInt32 inSize,
	UInt64 &ioPosition)
{
	UInt32	bytesWritten = 0;

	try {
		StMutex lock(mLock);

		this->SetPosition(eFrom_Start, ioPosition);
		bytesWritten = this->Write(inBuffer, inSize);
		ioPosition = this->GetPosition();
	}
	catch (...) { RETHROW_FS_(eWrite,mFileRef->GetName()); }

	return bytesWritten;
}

HL_End_Namespace_BigRedH
