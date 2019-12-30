/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "HotlineArchiveStruct.h"


class CMyBuffer
{
	public:
		CMyBuffer();
		virtual ~CMyBuffer();
		
		// returns the amount of data read
		// if inNeededSize, the buffer will fill up up to neededSize
		// (ie, if there is 100 bytes in it now, and in needed is 150, it will read up to 50 bytes)
		Uint32 WriteBuf(const void *inData, Uint32 inDataSize, Uint32 inNeededSize = max_Uint32);
		
		void *GetPtr()			{	return mBuf;	}
		Uint32 GetSize()		{	return mSize;	}
		void Clear()			{	mSize = 0;		}
		
	protected:
		TPtr mBuf;
		Uint32 mSize;
		Uint32 mMaxSize;
};

enum
{
	arcStatus_WaitingForData		= 0,
	arcStatus_ReceivingFiles		= 1,
	arcStatus_Complete				= 2
};

class CMyArchiveDecoder
{
	public:
		CMyArchiveDecoder();
		~CMyArchiveDecoder();
		
		// returns an arcStatus
		Uint32 ProcessData(const void *inData, Uint32 inDataSize);
		
		// stats
		Uint32 GetTotalSize()										{	return mArchive.totalSize;		}
		Uint32 GetProcessedSize()									{	return mProcessedSize;			}
		Uint32 GetTotalItems()										{	return mArchive.fileCount;		}
		Uint32 GetCurrentItem()										{	return mCurrentItem;			}
		Uint32 GetArchiveName(Uint8 *outData, Uint32 inMaxSize)		{	return UMemory::Copy(outData, mArchive.name+1, min((Uint32)mArchive.name[0], inMaxSize, (Uint32)(sizeof(mArchive.name) - 1)));	}
		Uint32 GetCurrentItemName(Uint8 *outData, Uint32 inMaxSize)	{	return mCurrentFile.ref ? mCurrentFile.ref->GetName(outData, inMaxSize) : 0;		}
		TFSRefObj* GetAutoLaunchRef()									{	return mArchive.autoLaunchRef;	}
		
	protected:
		// Process
		Uint32 ProcessHeader(const void *inData, Uint32 inDataSize);
		Uint32 ProcessPathHead(const void *inData, Uint32 inDataSize);
		Uint32 ProcessPath(const void *inData, Uint32 inDataSize);
		Uint32 ProcessFileRsvdHead(const void *inData, Uint32 inDataSize);
		Uint32 ProcessFileRsvd(const void *inData, Uint32 inDataSize);
		Uint32 ProcessFileHead(const void *inData, Uint32 inDataSize);
		Uint32 ProcessFile(const void *inData, Uint32 inDataSize);
		Uint32 ProcessFolder(const void *inData, Uint32 inDataSize);

		Uint32 mState;
		Uint32 mProcessedSize;	// compressed
		Uint32 mCurrentItem;
		
		CMyBuffer mBuf;
			
		struct
		{
			Uint32 totalSize;
			Uint32 fileCount;
			TFSRefObj* autoLaunchRef;
			Uint16 autoLaunchNum;
			Uint8 name[64];
		} mArchive;
		
		struct
		{
			TFSRefObj* ref;
			Uint32 type;
			Uint16 pathSize;
			Uint16 rsvdSize;
			Uint32 compressionType;
			Uint32 decompressedSize;
			Uint32 compressedSize;
			Uint32 processedSize;
			TZDecompress decompressor;
		} mCurrentFile;
};

