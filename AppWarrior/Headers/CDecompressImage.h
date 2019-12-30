/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "ImageTypes.h"

class CDecompressImage
{
	public:
		// constructor
		CDecompressImage(Uint32 inSize, bool inSilentFail, bool inBottomTopUpdate);
		virtual ~CDecompressImage();
				
		virtual bool Decompress(const void *inData, Uint32 inDataSize) = 0;
		
		bool GetSize(Uint32& outWidth, Uint32& outHeight);
		bool IsComplete();	
				
		virtual void Draw(TImage inDest, const SPoint& inDestPt, const SPoint& inSourcePt, Uint32 inWidth, Uint32 inHeight, Uint32 inOptions = 0);

		TImage GetImagePtr();
		TImage DetachImagePtr();

	protected:
		const void *mBuffer;   	// buffer source for image
		Uint32 mBufferSize; 	// buffer size 
		
		Uint32 mOffset;         // read offset
		Uint32 mBytesInBuffer;  // number of bytes in the buffer

		SPixmap mPixmap;
		TImage mImage;
		
		Uint32 mReadRows;
		Uint32 mUpdateRows;
		
		bool mSilentFail;
		bool mBottomTopUpdate;
		bool mFullUpdate;
		bool mIsComplete;
		
		void InitCompatibleImage();
		void UpdateImage();
		void ClearPixmap();
		Uint32 ReadImage(void *outBuf, Uint32 inBufSize, bool inExpectedEnd = false);
};

