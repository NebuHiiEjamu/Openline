/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
/*
-- Errors --

100		An unknown file system error has occured.
101		Not enough disk space.  Try deleting unwanted files to free up more space.
102		Specified file or folder does not exist.
103		Specified file does not exist.
104		Specified folder does not exist.
105		Specified disk does not exist.
106		Disk is damaged.
107		Disk software is damaged.
108		Disk hardware is damaged.
109		Disk is of unknown format.
110		Disk does not support operation.
111		File system item is locked and can't be changed.
112		Disk is locked or read-only and can't be changed.
113		File is locked and can't be changed.  Unlock the file and try again.
114		Folder is locked and can't be changed.  Unlock the folder and try again.
115		A file or folder with the specified name already exists.
116		Cannot open or delete file because it is in use (busy).
117		User does not have access to the specified file or folder.
118		User does not have permission to change the specified file or folder.
119		A bad file or folder name was specified.
120		The end of a file was encountered unexpectedly.
121		Disk is offline and can't be accessed.  If ejected, reinsert the disk and try again.
122		Cannot move folder into itself.
123		Disk unexpectedly vanished! (disconnected)
124		Expected file, but got folder.
125		Expected folder, but got file.
126		Expected different file/folder, but same file/folder was specified.
127		User is not authorized to perform operation.
128		Cannot resolve alias because the original item cannot be found.
129		Expected files/folders to be on the same disk (but they're not).
*/

#include "UFileSys.h"

/* -------------------------------------------------------------------------- */

Uint32 UFileSys::GetPathTargetName(const void *inPath, Uint32 inPathSize, void *outName, Uint32 inMaxSize, Uint32 *outEncoding, Uint8 **outPtr)
{
	Uint8 *p, *ep;
	Uint32 s, cnt;
	
	if (outPtr) *outPtr = nil;	// assume failure
	
	if (inPath == nil || inPathSize < 6)
		return 0;
	
	p = (Uint8 *)inPath;
	ep = ((Uint8 *)inPath) + inPathSize;
	cnt = FB( *(Uint16 *)p );
	p += sizeof(Uint16);

	// loop thru the names before the target (last one)
	if (cnt == 0) return 0;
	cnt--;
	while (cnt--)
	{
		p += sizeof(Uint16);
		if (p >= ep) return 0;		// check for corruption
		p += sizeof(Uint8) + *(Uint8 *)p;
	}
	
	// we're now up to the encoding for the target name
	if (outEncoding)
	{
		if ((p + sizeof(Uint16)) > ep) return 0;	// check for corruption
		*outEncoding = FB( *(Uint16 *)p );
	}
	p += sizeof(Uint16);
	if (p >= ep) return 0;			// check for corruption
	
	// copy name out
	s = *p++;
	if ((p + s) > ep) return 0;		// check for corruption
	if (s > inMaxSize) s = inMaxSize;
	UMemory::Copy(outName, p, s);
	
	// set ptr, must be done last (after corruption checks)
	if (outPtr) *outPtr = p-1;

	// all done, return size of name
	return s;
}


/*
 * GetPathSize() returns the size in bytes of the path data at <inPath>.
 * If <inOverrideCount> is less than the count in the path, <inOverrideCount>
 * is used instead.  Assumes the path data is not corrupt.
 */
Uint32 UFileSys::GetPathSize(const void *inPath, Uint16 inOverrideCount)
{
	Uint8 *p = (Uint8 *)inPath;
	Uint32 cnt;
	
	if (inPath == nil) return 0;
	
	cnt = FB( *(Uint16 *)p );
	p += sizeof(Uint16);
	
	if (inOverrideCount < cnt)
		cnt = inOverrideCount;

	while (cnt--)
	{
		p += sizeof(Uint16);
		p += sizeof(Uint8) + *(Uint8 *)p;
	}
	
	return p - (Uint8 *)inPath;
}


// modified ioPath to use valid file names.  the returned path size <= inPathSize
Uint32 UFileSys::ValidateFilePath(void *ioPath, Uint32 inPathSize)
{
	Uint8 outPath[2048];
	
	if (inPathSize > sizeof(outPath))
		Fail(errorType_Misc, error_OutOfRange);
	
	Uint8 *p = (Uint8 *)ioPath;
	Uint8 *ep = p + inPathSize;

	Uint8 *op = (Uint8 *)outPath;
	
	if (inPathSize < 2)
		return inPathSize;
	
	Uint16 count = FB(*((Uint16 *)p));
	
	*((Uint16 *)op)++ = *((Uint16 *)p)++;
	
	if (ep - p < count * (sizeof(Uint16) + 2 * sizeof(Uint8)))
		return inPathSize;
	
	while(count--)
	{
		*((Uint16 *)op)++ = *((Uint16 *)p)++;
		
		pstrcpy(op, p);

		Uint32 len = *p++;
		if (ep - p < len + count * (sizeof(Uint16) + 2 * sizeof(Uint8)))
			return inPathSize;
			
		p += len;
		op[0] = ValidateFileName(op + 1, len);
		op += op[0] + 1;
	}
	
	return UMemory::Copy(ioPath, outPath, op - outPath);

}

// converts path data (03??3MAC??6really??5sucks) to URL/is/the/best/ format
Uint32 UFileSys::ConvertPathDataToUrl(const void *inPath, Uint32 inPathSize, void *outData, Uint32 inMaxSize)
{
	if (!inPath || inPathSize < 6 || !outData || !inMaxSize)
		return 0;
	
	Uint32 nCount = FB( *(Uint16 *)inPath );
	Uint8 *p = ((Uint8 *)inPath) + sizeof(Uint16);
	Uint8 *ep = ((Uint8 *)inPath) + inPathSize;
	Uint8 *op = (Uint8 *)outData;
	
	while (nCount--)
	{
		p += sizeof(Uint16);
		if (p >= ep) break;					// check for corruption
		
		Uint32 nSize = *p++;
		if (nSize == 0 || p + nSize > ep) break;	// check for corruption
		if (op - (Uint8 *)outData + nSize + 1 > inMaxSize) break;
		
		// disallow going up a directory (for security reasons)
		if (nSize == 2 && p[0] == '.' && p[1] == '.')
		{
			p += 2;
			continue;
		}
		
		while (nSize--)
		{
			Uint8 c = *p++;
			*op++ = (c == '\\') ? '-' : c;
		}

		*op++ = '/';
	}
	
	return op - (Uint8 *)outData;
}

// converts URL/is/the/best/ format to path data (03??3MAC??6really??5sucks)
Uint32 UFileSys::ConvertUrlToPathData(const void *inPath, Uint32 inPathSize, void *outData, Uint32 inMaxSize)
{
	if (!inPath || !inPathSize || !outData || inMaxSize < 6)
		return 0;
	
	*((Uint16 *)outData) = 0;
	
	Uint8 *pStartF = (Uint8 *)inPath;
	Uint8 *pEndPath = (Uint8 *)inPath + inPathSize;
	
	Uint8 *pStartData = (Uint8 *)outData + 2;
	Uint8 *pEndData = (Uint8 *)outData + inMaxSize;
	
	while (*pStartF == '/')
		pStartF++;
		
	while(true)
	{
		if (pStartF >= pEndPath)
			break;
		
		Uint8 *pEndF = UMemory::SearchByte('/', pStartF, pEndPath - pStartF);
		
		if (!pEndF)
			pEndF = pEndPath;
		
		// disallow going up a directory (for security reasons)
		if (pEndF - pStartF == 2 && *pStartF == '.' && *(pStartF + 1) == '.')
		{
			pStartF += 3;
			continue;
		}
		
		if (pEndData - pStartData - 2 < pEndF - pStartF)
			break;
		
#if MACINTOSH
		(*(Uint16 *)outData)++;
#else
		*(Uint16 *)outData = TB( (Uint16)(FB(*(Uint16 *)outData) + 1));
#endif

		*((Uint16 *)pStartData) = 0;
		pStartData += 2;
		
		pStartData[0] = UMemory::Copy(pStartData + 1, pStartF, pEndF - pStartF > 255 ? 255 : pEndF - pStartF);

		pStartData += pStartData[0] + 1;
		pStartF = pEndF + 1;
	}
	
	return (pStartData - (Uint8 *)outData);
}

// size of ioPath must be inPathSize + inNameSize + 3 or inNameSize + 5 (if inPathSize is 0)
Uint32 UFileSys::AppendToPath(void *ioPath, Uint32 inPathSize, const Uint8 *inName, Uint32 inNameSize, Uint32 inNameEncoding)
{
	Uint8 *p = BPTR(ioPath);
	
	// write the count
	if(inPathSize)
	{
		*((Uint16 *)p) = TB((Uint16)(FB(*((Uint16 *)ioPath)) + 1));
		p += inPathSize;
	}
	else
	{
		*((Uint16 *)p)++ = TB((Uint16)1);
	}
		
	// write the encoding
	*((Uint16 *)p)++ = TB((Uint16)inNameEncoding);
	
	// write the name;
	*p = UMemory::Copy(p + 1, inName, (Uint8)inNameSize);
	p+= *p + 1;
	
	return p - BPTR(ioPath);
}

void *UFileSys::MakePathData(const void *inPath, Uint32 inPathSize, const Uint8 *inName, Uint32& outPathSize)
{
	outPathSize = 0;

	if (!inName || !inName[0])
		return nil;
	
	if (inPath && inPathSize)
		outPathSize = inPathSize + inName[0] + 3;
	else
		outPathSize = inName[0] + 5;
	
	void *outPath = UMemory::NewClear(outPathSize);
	Uint8 *pPath = (Uint8 *)outPath;

	if (inPath && inPathSize)
		UMemory::Copy(pPath, inPath, inPathSize);

#if MACINTOSH
		(*(Uint16 *)pPath)++;
#else
		*(Uint16 *)pPath = TB( (Uint16)(FB(*(Uint16 *)pPath) + 1));
#endif

	if (inPath && inPathSize)
		pPath += inPathSize + 2;
	else
		pPath += 4;
		
	UMemory::Copy(pPath, inName, inName[0] + 1);

	return outPath;
}

void *UFileSys::MakePathData(const void *inPath1, Uint32 inPathSize1, const void *inPath2, Uint32 inPathSize2, Uint32& outPathSize)
{
	outPathSize = 0;
	
	if (!inPath1 || inPathSize1 < 6 || !inPath2 || inPathSize2 < 6)
		return nil;
	
	Uint8 *pStartPath1 = (Uint8 *)inPath1;
	Uint8 *pStartPath2 = (Uint8 *)inPath2;

	outPathSize = inPathSize1 + inPathSize2 - 2;
	void *outPath = UMemory::New(outPathSize);
	Uint8 *pPath = (Uint8 *)outPath;

#if MACINTOSH
	*((Uint16 *)pPath) = *(Uint16 *)pStartPath1 + *(Uint16 *)pStartPath2;
#else
	*((Uint16 *)pPath) = TB((Uint16)(FB(*(Uint16 *)pStartPath1) + FB(*(Uint16 *)pStartPath2)));
#endif

	pPath += 2;

	pStartPath1 += 2;
	pPath += UMemory::Copy(pPath, pStartPath1, inPathSize1 - 2);
	
	pStartPath2 += 2;
	UMemory::Copy(pPath, pStartPath2, inPathSize2 - 2);

	return outPath;
}

void *UFileSys::GetParentPathData(const void *inPath, Uint32 inPathSize, Uint32& outPathSize)
{
	outPathSize = 0;
	
	if (!inPath || !inPathSize)
		return nil;
		
	Uint32 nPathSize = UFileSys::GetPathSize(inPath, FB(*(Uint16 *)inPath) - 1);
	if (nPathSize <= 2)
		return nil;
		
	outPathSize = nPathSize;
	void *pPath = UMemory::New(inPath, outPathSize);

#if MACINTOSH
	(*(Uint16 *)pPath)--;
#else
	*(Uint16 *)pPath = TB( (Uint16)(FB(*(Uint16 *)pPath) - 1) );
#endif

	return pPath;
}

#pragma mark -

Uint32 CTempFile::mTempCount = 0;

CTempFile::CTempFile()
{
	mFile = nil;
	mFileName[0] = 0;
	mTempFileName[0] = 0;
	mFileInfo[0] = 0;
	
	mTypeCode = 'BINA';
	mCreatorCode = 'dosa';
	mChecksum = 0;
}
	
CTempFile::~CTempFile()
{			
	if (mFile)
	{
		try
		{
			if (mFile->Exists())
				mFile->DeleteFile();	// DeleteFile trow an exception if the file is busy
		}
		catch(...)
		{
			// even if DeleteFile fails we still want to destruct this object
		}
		
		UFileSys::Dispose(mFile);
		mFile = nil;
	}
}

bool CTempFile::SetTempFile(const Uint8 *inFileName, Uint32 inTypeCode, Uint32 inCreatorCode, bool inDefault)
{
	DeleteFile();

	if (!inFileName || !inFileName[0])
		return false;

	UMemory::Copy(mFileName, inFileName, inFileName[0] + 1);

	mTypeCode = inTypeCode;
	mCreatorCode = inCreatorCode;
			
	Uint8 psExtension[8];
	if (inDefault)
		psExtension[0] = UMemory::Copy(psExtension + 1, ".tmp", 4);
	else
	{
		Uint32 nExtensionSize = 0;
		const Uint8 *pExtension = UMemory::SearchByte('.', inFileName + 1, inFileName[0]);
		
		if (pExtension)
		{
			nExtensionSize = inFileName + 1 + inFileName[0] - pExtension;
			if (nExtensionSize > sizeof(psExtension) - 1)
				nExtensionSize = sizeof(psExtension) - 1;
		}
		else
		{
			const Int8 *pMime = UMime::ConvertTypeCode_Mime(inTypeCode);
			pExtension = (Uint8 *)UMime::ConvertMime_Extension(pMime);
			
			nExtensionSize = strlen((Int8 *)pExtension);
		}

		psExtension[0] = UMemory::Copy(psExtension + 1, pExtension, nExtensionSize);
	}
	
	do
	{
		if (mFile)
			UFileSys::Dispose(mFile);
		
		mTempFileName[0] = UText::Format(mTempFileName + 1, sizeof(mTempFileName) - 1, "HTLC%hu%#s", CTempFile::mTempCount++, psExtension);
		mFile = UFS::New(kTempFolder, nil, mTempFileName);

	} while(mFile->Exists());
		
	
	Uint32 nTypeCode;
	if (inDefault)
		nTypeCode = 'BINA';
	else
		nTypeCode = mTypeCode;
	
	mFile->CreateFile(nTypeCode, 'HTLC');
	
	return true;
}

void CTempFile::SetFileInfo(const Uint8 *inFileInfo)
{
	if (!inFileInfo || !inFileInfo[0])
		return;

	Uint8 nSize = inFileInfo[0];
	if (nSize > sizeof(mFileInfo) - 1) 
		nSize = sizeof(mFileInfo) - 1;
		
	mFileInfo[0] = UMemory::Copy(mFileInfo + 1, inFileInfo + 1, nSize);
}

TFSRefObj* CTempFile::GetFile()
{
	return mFile;
}

TFSRefObj* CTempFile::GetCloneFile()
{
	if (mFile)
		return mFile->Clone();

	return nil;
}

bool CTempFile::SaveFileAs(TFSRefObj* inInitianFolder)
{
	if (!mFile || !mFile->Exists())
		return false;
			
	Uint8 *psFileName = nil;
	if (mFileName[0] != 0)
		psFileName = mFileName;
				
	TFSRefObj* pNewFile = UFS::UserSaveFile(inInitianFolder, psFileName);
	if (!pNewFile)
		return false;
		
	scopekill(TFSRefObj, pNewFile);

	if (pNewFile->Exists())
		pNewFile->DeleteFile();
	
	return UFileSys::Copy(mFile, pNewFile, mTypeCode, mCreatorCode);
}

bool CTempFile::GetFileName(Uint8 *outFileName, Uint32 inMaxSize)
{
	if (!mFileName[0] || !outFileName || !inMaxSize)
		return false;
	
	outFileName[0] = UMemory::Copy(outFileName + 1, mFileName + 1, mFileName[0] > inMaxSize - 1 ? inMaxSize - 1 : mFileName[0]);
	return true; 
}

bool CTempFile::GetTempName(Uint8 *outTempFileName, Uint32 inMaxSize)
{
	if (!mTempFileName[0] || !outTempFileName || !inMaxSize)
		return false;
	
	outTempFileName[0] = UMemory::Copy(outTempFileName + 1, mTempFileName + 1, mTempFileName[0] > inMaxSize - 1 ? inMaxSize - 1 : mTempFileName[0]);
	return true;
}

Uint32 CTempFile::GetTypeCode()
{
	return mTypeCode;
}

Uint32 CTempFile::GetTempTypeCode()
{
	if (!mFile || !mFile->Exists())
		return 0;
	
	return mFile->GetTypeCode();
}

Uint32 CTempFile::GetCreatorCode()
{
	return mCreatorCode;
}

Uint32 CTempFile::GetTempCreatorCode()
{
	if (!mFile || !mFile->Exists())
		return 0;
	
	return mFile->GetCreatorCode();
}

Uint32 CTempFile::GetChecksum()
{
	return mChecksum;
}

Uint32 CTempFile::GetFileSize()
{
	if (!mFile || !mFile->Exists())
		return 0;
	
	return mFile->GetSize();
}

Uint32 CTempFile::GetFileInfo(Uint8 *outFileInfo, Uint32 inMaxSize)
{
	if (!mFileInfo[0] || !outFileInfo || !inMaxSize)
		return 0;
	
	return UMemory::Copy(outFileInfo, mFileInfo + 1, mFileInfo[0] > inMaxSize ? inMaxSize : mFileInfo[0]);
}

bool CTempFile::WriteFileData(const void *inBuffer, Uint32 inBufferSize)
{
	if (!inBuffer || !inBufferSize || !mFile || !mFile->Exists())
		return false;
		
	StFileOpener fileOpener(mFile, perm_Wr);
	if (mFile->Write(mFile->GetSize(), inBuffer, inBufferSize) != inBufferSize)
		return false;
	
	mChecksum = UMemory::Checksum(inBuffer, inBufferSize, mChecksum);
	return true;
}

bool CTempFile::ReadFileData(void *outBuffer, Uint32 inBufferSize)
{
	return ReadFileData(0, outBuffer, inBufferSize);
}

bool CTempFile::ReadFileData(Uint32 inOffset, void *outBuffer, Uint32 inBufferSize)
{
	if (!outBuffer || !inBufferSize || !mFile || !mFile->Exists())
		return false;

	StFileOpener fileOpener(mFile, perm_Rd);
	if (mFile->Read(inOffset, outBuffer, inBufferSize) != inBufferSize)
		return false;
	
	return true;
}

THdl CTempFile::ReadFileData(Uint32& outTypeCode)
{
	outTypeCode = 0;
			
	if (!mFile || !mFile->Exists())
		return false;
	
	outTypeCode = mTypeCode;
		
	Uint32 nBufferSize = mFile->GetSize();
	StFileOpener fileOpener(mFile, perm_Read);
	return mFile->ReadToHdl(0, nBufferSize);
}

bool CTempFile::CompareFileName(const Uint8 *inFileName, Uint8 *outTempFileName)
{
	if (!mFileName[0] || !inFileName || !inFileName[0])
		return false;
	
	if (!UText::CompareInsensitive(mFileName + 1, mFileName[0], inFileName + 1, inFileName[0]))
	{
		if (outTempFileName)
			UMemory::Copy(outTempFileName, mTempFileName, mTempFileName[0] + 1);
		
		return true;
	}
	
	return false;
}

bool CTempFile::CompareTempFileName(const Uint8 *inTempFileName, Uint8 *outFileName)
{
	if (!mTempFileName[0] || !inTempFileName || !inTempFileName[0])
		return false;
	
	if (!UText::CompareInsensitive(mTempFileName + 1, mTempFileName[0], inTempFileName + 1, inTempFileName[0]))
	{
		if (outFileName)
			UMemory::Copy(outFileName, mFileName, mFileName[0] + 1);
		
		return true;
	}
	
	return false;
}

bool CTempFile::IsValidFile()
{
	if (!mFile || !mFile->Exists())
		return false;
	
	return true;
}

void CTempFile::DeleteFile()
{			
	if (mFile)
	{
		try
		{
			if (mFile->Exists())
				mFile->DeleteFile();	// DeleteFile trow an exception if the file is busy
		}
		catch(...)
		{
			// even if DeleteFile fails we still want to clear this object
		}
			
		UFileSys::Dispose(mFile);
		mFile = nil;
	}

	mFileName[0] = 0;
	mTempFileName[0] = 0;
	mFileInfo[0] = 0;

	mTypeCode = 'BINA';
	mCreatorCode = 'dosa';
	mChecksum = 0;
}

void CTempFile::MoveToTrash()
{			
	if (mFile)
	{
		try
		{
			if (mFile->Exists())
				mFile->MoveToTrash();
		}
		catch(...)
		{
			// even if MoveToTrash fails we still want to clear this object
		}
			
		UFileSys::Dispose(mFile);
		mFile = nil;
	}
	
	mFileName[0] = 0;
	mTempFileName[0] = 0;
	mFileInfo[0] = 0;

	mTypeCode = 'BINA';
	mCreatorCode = 'dosa';
	mChecksum = 0;
}


#pragma mark -

CEncodedTempFile::CEncodedTempFile()
	: CTempFile()
{
	mWasEncoded = false;
	
	mFileSize = 0;
	ClearStruct(mCreatedDate);

	mFileCrcStart = 0;
	mFileCrc = 0;
		
	mFileAdlerStart = 0;
	mFileAdler = 0;
}
	
CEncodedTempFile::~CEncodedTempFile()
{
	DeleteFile();
}

bool CEncodedTempFile::SetTempFile(const Uint8 *inFileName, Uint32 inTypeCode, Uint32 inCreatorCode, bool inDefault)
{
	if (!CTempFile::SetTempFile(inFileName, inTypeCode, inCreatorCode, inDefault))
		return false;
	
	mFile->GetDateStamp(nil, &mCreatedDate);
	return true;
}

bool CEncodedTempFile::WriteFileData(const void *inBuffer, Uint32 inBufferSize)
{
	if (!inBuffer || !inBufferSize)
		return false;
	
	mFileSize += inBufferSize;

	if (!mWasEncoded)
		return EncodeFileData(inBuffer, inBufferSize);
	else
		return CTempFile::WriteFileData(inBuffer, inBufferSize);
}

bool CEncodedTempFile::EncodeFileData(const void *inBuffer, Uint32 inBufferSize)
{
	if (!mFile || !mFile->Exists())
		return false;
	
	EncodeBuffer(inBuffer, inBufferSize);
	CTempFile::WriteFileData(inBuffer, inBufferSize);
		
	mFileCrcStart = UMath::GetRandom(0, inBufferSize - 20);
	mFileCrc = UMemory::CRC((Uint8 *)inBuffer + mFileCrcStart, 20, -2);
		
	mFileAdlerStart = UMath::GetRandom(0, inBufferSize - 20);
	mFileAdler = UMemory::AdlerSum((Uint8 *)inBuffer + mFileAdlerStart, 20, 2);
	
	DecodeBuffer(inBuffer, inBufferSize);
	mWasEncoded = true;
	
	return true;
}

THdl CEncodedTempFile::ReadFileData(Uint32& outTypeCode)
{
	THdl hBuffer = CTempFile::ReadFileData(outTypeCode);
	
	if (!hBuffer)
		return nil;
		
	void *pBuffer = hBuffer->Lock();
	DecodeBuffer(pBuffer, mFile->GetSize());
	hBuffer->Unlock();
	
	return hBuffer;
}

bool CEncodedTempFile::IsValidFile()
{
	if (!CTempFile::IsValidFile() || mFile->GetSize() != mFileSize)
		return false;

	SDateTimeStamp stCreatedDate, stModifiedDate;
	mFile->GetDateStamp(&stModifiedDate, &stCreatedDate);

	if (mCreatedDate != stCreatedDate)
		return false;

	Uint8 bufCrcAdler[20];
	StFileOpener fileOpener(mFile, perm_Read);

	mFile->Read(mFileCrcStart, bufCrcAdler, 20);
	Uint32 nFileCrc = UMemory::CRC(bufCrcAdler, 20, -2);
	
	if (mFileCrc != nFileCrc)
		return false;
		
	mFile->Read(mFileAdlerStart, bufCrcAdler, 20);
	Uint32 nFileAdler = UMemory::AdlerSum(bufCrcAdler, 20, 2);
	
	if (mFileAdler != nFileAdler)
		return false;
	
	return true;
}

void CEncodedTempFile::DeleteFile()
{			
	mWasEncoded = false;

	ClearStruct(mCreatedDate);

	mFileCrcStart = 0;
	mFileCrc = 0;

	mFileAdlerStart = 0;
	mFileAdler = 0;	

	CTempFile::DeleteFile();
}

inline void CEncodedTempFile::EncodeBuffer(const void *inBuffer, Uint32 inBufferSize)
{	
	Uint8 *pBuffer = (Uint8 *)inBuffer;
	
	if (mTypeCode == TB('PICT'))
		pBuffer += 512;
	
	for (Uint8 i=0; i<25; i++)
	{	
		if(i*2 + pBuffer - inBuffer < inBufferSize)
			if (i%2) pBuffer[i*2] += (i + 2)*4;
			else	 pBuffer[i*2] -= (i + 4)*6; 
		
		mEncodingOffset[i] = UMath::GetRandom(0, inBufferSize - (pBuffer - inBuffer) - 1);
		Uint8 *pEncodingByte = pBuffer + mEncodingOffset[i];
		
		if (i%3) *pEncodingByte += (i + 3)*5;
		else 	 *pEncodingByte -= (i + 5)*7;
	}
}
 
inline void CEncodedTempFile::DecodeBuffer(const void *inBuffer, Uint32 inBufferSize)
{
	Uint8 *pBuffer = (Uint8 *)inBuffer;
	
	if (mTypeCode == TB('PICT'))
		pBuffer += 512;

	for (Uint8 i=0; i<25; i++)
	{
		Uint8 j = 24 - i;
		
		if (j*2 + pBuffer - inBuffer < inBufferSize)
			if (j%2) pBuffer[j*2] -= (j + 2)*4;
			else	 pBuffer[j*2] += (j + 4)*6; 
		
		Uint8 *pDecodingByte = pBuffer + mEncodingOffset[j];
		
		if (j%3) *pDecodingByte -= (j + 3)*5;
		else	 *pDecodingByte += (j + 5)*7;
	}
}


#pragma mark -

CPurgedTempFile::CPurgedTempFile(Uint32 inMaxFileSize)
	: CTempFile()
{
	mMaxFileSize = inMaxFileSize;
	mWriteOffset = 0;
	mReadOffset = 0;
}
	
CPurgedTempFile::~CPurgedTempFile()
{
	DeleteFile();
}

Uint32 CPurgedTempFile::GetWriteSize()
{
	return mWriteOffset >= mReadOffset ? mMaxFileSize - mWriteOffset + mReadOffset : mReadOffset - mWriteOffset;
}

bool CPurgedTempFile::CanWriteSize(Uint32 inBufferSize)
{
	if (inBufferSize > (mWriteOffset >= mReadOffset ? mMaxFileSize - mWriteOffset + mReadOffset : mReadOffset - mWriteOffset))
		return false;
		
	return true;
}

bool CPurgedTempFile::WriteFileData(const void *inBuffer, Uint32 inBufferSize)
{
	if (!inBuffer || !inBufferSize || !mFile || !mFile->Exists() || !CanWriteSize(inBufferSize))
		return false;
		
	Uint32 nWriteSize = inBufferSize;
	if (mWriteOffset >= mReadOffset && inBufferSize > mMaxFileSize - mWriteOffset)
		nWriteSize =  mMaxFileSize - mWriteOffset;
	
	StFileOpener fileOpener(mFile, perm_ReadWrite);
	if (mFile->Write(mWriteOffset, inBuffer, nWriteSize) != nWriteSize)
		return false;
	
	mWriteOffset += nWriteSize;
	mChecksum = UMemory::Checksum(inBuffer, nWriteSize, mChecksum);
	
	if (nWriteSize < inBufferSize)
	{
		Uint32 nExtraWriteSize = inBufferSize - nWriteSize;
		
		if (mFile->Write(0, (Uint8 *)inBuffer + nWriteSize, nExtraWriteSize) != nExtraWriteSize)
			return false;
		
		mWriteOffset = nExtraWriteSize;
		mChecksum = UMemory::Checksum((Uint8 *)inBuffer + nWriteSize, nExtraWriteSize, mChecksum);
	}
	
	return true;
}

Uint32 CPurgedTempFile::GetReadSize()
{
	return mWriteOffset >= mReadOffset ? mWriteOffset - mReadOffset : mMaxFileSize - mReadOffset + mWriteOffset;
}

bool CPurgedTempFile::CanReadSize(Uint32 inBufferSize)
{
	if (inBufferSize > (mWriteOffset >= mReadOffset ? mWriteOffset - mReadOffset : mMaxFileSize - mReadOffset + mWriteOffset))
		return false;
		
	return true;
}

bool CPurgedTempFile::ReadFileData(void *outBuffer, Uint32 inBufferSize)
{
	if (!outBuffer || !inBufferSize || !mFile || !mFile->Exists() || !CanReadSize(inBufferSize))
		return false;

	Uint32 nReadSize = inBufferSize;
	if (mWriteOffset < mReadOffset && inBufferSize > mMaxFileSize - mReadOffset)
		nReadSize =  mMaxFileSize - mReadOffset;

	StFileOpener fileOpener(mFile, perm_ReadWrite);
	if (mFile->Read(mReadOffset, outBuffer, nReadSize) != nReadSize)
		return false;
	
	mReadOffset += nReadSize;

	if (nReadSize < inBufferSize)
	{
		Uint32 nExtraReadSize = inBufferSize - nReadSize;
		
		if (mFile->Read(0, (Uint8 *)outBuffer + nReadSize, nExtraReadSize) != nExtraReadSize)
			return false;
		
		mReadOffset = nExtraReadSize;
	}

	return true;
}


#pragma mark -

CGeneralFile::CGeneralFile(bool inPurge)
	: CTempFile()
{
	mPurge = inPurge;
}
	
CGeneralFile::~CGeneralFile()
{
	if (mPurge)
		MoveToTrash();
}

void CGeneralFile::SetGeneralFile(TFSRefObj* inFolder, Uint8 *inFileName, Uint32 inTypeCode, Uint32 inCreatorCode, bool inCreateNew)
{
	if (!inFolder || !inFileName || !inFileName[0])
		return;
	
	UMemory::Copy(mFileName, inFileName, inFileName[0] + 1);
	
	mTypeCode = inTypeCode;
	mCreatorCode = inCreatorCode;
	
	if (mFile)
	{
		UFileSys::Dispose(mFile);
		mFile = nil;
	}
	
	mFile = UFS::New(inFolder, nil, mFileName);
		
	if (inCreateNew)
	{
	 	if (mFile->Exists())
			mFile->MoveToTrash();
	
		mFile->CreateFile('BINA', 'HTLC');
	}
}

bool CGeneralFile::SaveFileAs(TFSRefObj* inInitianFolder)
{	 	
	if (!mFile || !mFile->Exists())
		return false;
			
	Uint8 *psFileName = nil;
	if (mFileName[0] != 0)
		psFileName = mFileName;
				
	TFSRefObj* pNewFile = UFS::UserSaveFile(inInitianFolder, psFileName);
	if (!pNewFile)
		return false;
		
	scopekill(TFSRefObj, pNewFile);

	if (pNewFile->Exists())
		pNewFile->DeleteFile();
				
	return UFileSys::Copy(mFile, pNewFile, mTypeCode, mCreatorCode);
}

void CGeneralFile::DeleteFile()
{			
	mPurge = false;

	CTempFile::DeleteFile();
}

void CGeneralFile::MoveToTrash()
{			
	mPurge = false;

	CTempFile::MoveToTrash();
}

#pragma mark -

Uint32 _FSTypeCodeToCreatorCode(Uint32 inTypeCode)
{
	Uint32 nCreatorCode;
	
	switch (inTypeCode)
	{
// misc
		case 'TEXT':	nCreatorCode = 'ttxt';	break;
		case 'SITD':	nCreatorCode = 'SIT!';	break;
		case 'BINA':	nCreatorCode = 'hDmp';	break;
		case 'DEXE':	nCreatorCode = 'CWIE';	break;
		case 'ZIP ':	nCreatorCode = 'ZIP ';	break;
		case 'W6BN':	nCreatorCode = 'MSWD';	break;
		case 'HTft':	nCreatorCode = 'HTLC';	break;
		case 'HTbm':	nCreatorCode = 'HTLC';	break;
		case 'HLNZ':	nCreatorCode = 'HTLC';	break;
// images
		case 'GIFf':	nCreatorCode = 'GKON';	break;
		case 'JPEG':	nCreatorCode = 'GKON';	break;
		case 'BMP ':	nCreatorCode = 'GKON';	break;
		case 'PICT':	nCreatorCode = 'GKON';	break;
		case 'FPix':	nCreatorCode = 'TVOD';	break;
		case 'PNTG':	nCreatorCode = 'TVOD';	break;
		case '8BPS':	nCreatorCode = 'TVOD';	break;
		case 'PNGf':	nCreatorCode = 'TVOD';	break;
		case 'SGI ':	nCreatorCode = 'TVOD';	break;
		case 'TPIC':	nCreatorCode = 'TVOD';	break;
		case 'TIFF':	nCreatorCode = 'GKON';	break;	
// video
		case 'MooV':	nCreatorCode = 'TVOD';	break;
		case 'MPEG':	nCreatorCode = 'TVOD';	break;
		case 'VfW ':	nCreatorCode = 'TVOD';	break;
		case 'dvc!':	nCreatorCode = 'TVOD';	break;
// 3D
		case 'SWFL':	nCreatorCode = 'SWF2';	break;
		case 'FLI ':	nCreatorCode = 'TVOD';	break;
		case '3DMF':	nCreatorCode = 'TVOD';	break;
// audio	
		case 'MP3 ':	nCreatorCode = 'MAmp';	break;
		case 'RAE ':	nCreatorCode = 'REAL';	break;
		case 'AIFF':	nCreatorCode = 'TVOD';	break;
		case 'ULAW':	nCreatorCode = 'TVOD';	break;
		case 'WAVE':	nCreatorCode = 'TVOD';	break;
		case 'Sd2f':	nCreatorCode = 'TVOD';	break;
		case 'MiDi':	nCreatorCode = 'TVOD';	break;

		default:		nCreatorCode = 'dosa';	break;
	};
	
	return nCreatorCode;
}
