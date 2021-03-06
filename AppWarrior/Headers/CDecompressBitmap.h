/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "CDecompressImage.h"


class CDecompressBitmap : public CDecompressImage
{
	public:
		// constructor
		CDecompressBitmap(Uint32 inSize, bool inSilentFail = false);
		virtual ~CDecompressBitmap();
		
		virtual bool Decompress(const void *inData, Uint32 inDataSize);

	protected:
		Uint8 mStage;
		BITMAPRGB *mColorBuf;
		
		bool ReadBitmapImage();
};
