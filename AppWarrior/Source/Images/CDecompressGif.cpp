/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "CDecompressGif.h"

#define NO_CODE		    -1


CDecompressGif::CDecompressGif(Uint32 inSize, bool inSilentFail) 
	: CDecompressImage(inSize, inSilentFail, false)
{
	ClearStruct(mGifHeader);      		
	ClearStruct(mGifInfo);
	ClearStruct(mImageInfo);
	ClearStruct(mImgBlock);        		
	ClearStruct(mCtrBlock);

	mIsCompleteImage = false;

	mStage = 1;
	mImageStage = 0;
	mUnpackStage = 0;
	mExtensionStage = 0;
	
	mLineBuffer = nil;
}

CDecompressGif::~CDecompressGif()
{
	if (mLineBuffer)
		UMemory::Dispose((TPtr)mLineBuffer);
}

bool CDecompressGif::Decompress(const void *inData, Uint32 inDataSize)
{
	mBuffer = inData;
	mBytesInBuffer = inDataSize;

	if (!mIsComplete && mIsCompleteImage)
	{
		ClearPixmap();
		mIsCompleteImage = false;
	}
	
	try
	{
		return ReadGifImage();
	}
	catch (SError& inError)
	{
		if (mLineBuffer)
		{
			UMemory::Dispose((TPtr)mLineBuffer);
			mLineBuffer = nil;
		}
		
		if (mSilentFail)
			inError.silent = true;
	
		throw; 
	}
	
	return false;
}

bool CDecompressGif::GetTransColor(Uint32& outTransColor)		
{							
	if (!(mCtrBlock.flags & 0x01))
	{
		outTransColor = 0;
		return false;
	}
	
	outTransColor = mPixmap.colorTab[mCtrBlock.transparent_colour];
	return true;
}		

TImage CDecompressGif::GetTransMask()
{
	if (!(mCtrBlock.flags & 0x01) || !mPixmap.width || !mPixmap.height)
		return nil;

	SPixmap stTransPixmap;
	stTransPixmap.width = mPixmap.width;
	stTransPixmap.height = mPixmap.height;
	stTransPixmap.depth = mPixmap.depth;                     				
	stTransPixmap.rowBytes = mPixmap.rowBytes;
	stTransPixmap.colorCount = 2;

	try
	{
		stTransPixmap.colorTab = (Uint32 *)UMemory::NewClear(stTransPixmap.colorCount * 4);
	}
	catch(...)
	{
		Fail(errorType_Image, imgError_InsufficientMemory);
	}
	
	Uint8 *pColorTab = (Uint8 *)stTransPixmap.colorTab;	
	for (Uint16 i=0; i<stTransPixmap.colorCount; i++)
	{
		*pColorTab++ = i*0xFF;   //Red
		*pColorTab++ = i*0xFF;   //Green
		*pColorTab++ = i*0xFF;   //Blue
		*pColorTab++ = 0;
	}

	try
	{
		stTransPixmap.data = UMemory::NewClear(stTransPixmap.rowBytes * stTransPixmap.height);
	}
	catch(...)
	{
		UMemory::Dispose((TPtr)stTransPixmap.colorTab);
		Fail(errorType_Image, imgError_InsufficientMemory);
	}

	Uint8 *pSource = (Uint8 *)mPixmap.data;
	Uint8 *pDest = (Uint8 *)stTransPixmap.data;

	for (Uint32 i = 0; i< stTransPixmap.height; i++)
		for (Uint32 j = 0; j< stTransPixmap.width; j++)
		{
			if (*(pSource + i*stTransPixmap.width + j) == mCtrBlock.transparent_colour)
				*(pDest + i*stTransPixmap.width + j) = 1;
		}
	
	TImage pTransMask = UGraphics::NewCompatibleImage(stTransPixmap.width, stTransPixmap.height);
	if (!pTransMask)
	{
		UMemory::Dispose((TPtr)stTransPixmap.colorTab);
		UMemory::Dispose((TPtr)stTransPixmap.data);
		Fail(errorType_Image, imgError_InsufficientMemory);
	}

	StImageLocker lockMask(pTransMask);
	UGraphics::CopyPixels(pTransMask, SPoint(0,0), stTransPixmap, SPoint(0,0), stTransPixmap.width, stTransPixmap.height);
	
	UMemory::Dispose((TPtr)stTransPixmap.colorTab);
	UMemory::Dispose((TPtr)stTransPixmap.data);
	
	return pTransMask;
} 

bool CDecompressGif::ReadGifImage()
{
	Uint32 nOldReadRows = mReadRows;

startagain:

	switch (mStage)
	{
		case 1:
			if (ReadImage(&mGifHeader, sizeof(GIFHEADER)) != END_OF_BYTES)
			{
				// make sure it's a GIF file
				if (UMemory::Compare(mGifHeader.sig, "GIF", 3)) 
					Fail(errorType_Image, imgError_NotGifFile);

				#if !CONVERT_INTS
				mGifHeader.screenwidth = swap_int(mGifHeader.screenwidth);
				mGifHeader.screendepth = swap_int(mGifHeader.screendepth);
				#endif

				// get screen dimensions
				mGifInfo.width = mGifHeader.screenwidth;
				mGifInfo.depth = mGifHeader.screendepth;
				mGifInfo.bits = (mGifHeader.flags & 0x0007) + 1;
				mGifInfo.flags = mGifHeader.flags;
				mGifInfo.background = mGifHeader.background;
				
				mStage = 2;
				goto startagain;
			}			
			break;
				
		case 2:
			// get colour map if there is one 
			if (mGifInfo.flags & 0x80) 
			{
				Uint16 palsize = 3 * (1 << mGifInfo.bits);
				if(ReadImage(mGifInfo.palette, palsize) != END_OF_BYTES)
				{
					mStage = 3;
					mImageStage = 1;
					goto startagain;
				}
			}
			else
			{
				mStage = 3;
				mImageStage = 1;
				goto startagain;
			}
			break;
			
		case 3:

imageagain:
			// step through the blocks 
			switch (mImageStage)
			{
				case 1:
					Uint8 ch;
					Uint32 nRet = ReadImage(&ch, 1, true);
					if (nRet == END_OF_FB)
					{
						mStage = 4;
						goto startagain;
					}
				
					if (nRet != END_OF_BYTES)
					{
						if ((ch!=',' && ch!='!' && ch!=0))
						{
							mStage = 4;
							goto startagain;
						}
					
						// if it's an image block... 
						if (ch == ',')
							mImageStage = 2;
					
						// if it's an extension 
						if (ch == '!')
						{	
							mImageStage = 6;
							mExtensionStage = 0x10;
						}
						
						goto imageagain;
					}
					break;
			
				case 2:
					// get the start of the image block 
					if (ReadImage(&mImgBlock, sizeof(IMAGEBLOCK)) != END_OF_BYTES)
					{
						#if !CONVERT_INTS
						mImgBlock.left = swap_int(mImgBlock.left);
						mImgBlock.top = swap_int(mImgBlock.top);
						mImgBlock.width = swap_int(mImgBlock.width);
						mImgBlock.depth = swap_int(mImgBlock.depth);
						#endif

						// get the image dimensions 
						mImageInfo.width = mImgBlock.width;
						mImageInfo.depth = mImgBlock.depth;
						mImageInfo.bits = (mImgBlock.flags & 0x0007) + 1;
						mImageInfo.flags = mImgBlock.flags;

						mImageStage = 3;
						goto imageagain;
					}
					break;
			
				case 3:
					// get the local colour map if there is one 
					if (mImageInfo.flags & 0x80) 
					{
						Uint16 palsize = 3 * (1 << mImageInfo.bits);
						if (ReadImage(mImageInfo.palette, palsize) != END_OF_BYTES)
						{
							mImageStage = 4;
							goto imageagain;
						}
					}
					else
					{
						mImageStage = 4;
						goto imageagain;
					}
					break;

				case 4:
					// get the initial code size 
					if (ReadImage(&mCodeSize, 1) != END_OF_BYTES)
					{
						InitPixmap();
						AllocMemory();

						mImageStage = 5;
						mUnpackStage = 1;
						goto imageagain;
					}
					break;

				case 5:
					// unpack the image 
					if (UnpackImage(mCodeSize))
					{
						mIsCompleteImage = true;
						
						mImageStage = 1;
						// don't goto imageagain;  
					}
					break;

				case 6:
					// put extension 
					if (PutExtension())
					{
						mImageStage = 1;
						goto imageagain;
					}
			};
			break;

		case 4: 
			mIsComplete = true;
			break;	
	};
	
	return (mReadRows > nOldReadRows);
}

void CDecompressGif::InitPixmap()
{
	mPixmap.width = mImageInfo.width;
	mPixmap.height = mImageInfo.depth;
	mPixmap.depth = 8;                     				
	mPixmap.rowBytes = mImageInfo.width;
	mPixmap.colorCount = 0;

	// get a local or global colour map if there is one
 	if ((mGifInfo.flags & 0x80) || (mImageInfo.flags & 0x80)) 
 	{
		Uint8* palette;
		if (mImageInfo.flags & 0x80)
		{
			mPixmap.colorCount = 1 << mImageInfo.bits;
			palette = mImageInfo.palette;
		}
		else
		{
			mPixmap.colorCount = 1 << mGifInfo.bits;
			palette = mGifInfo.palette;
		}

		if (mPixmap.colorTab)
			UMemory::Dispose((TPtr)mPixmap.colorTab);

		try
		{
			mPixmap.colorTab = (Uint32 *)UMemory::NewClear(mPixmap.colorCount * 4);
		}
		catch(...)
		{
			Fail(errorType_Image, imgError_InsufficientMemory);
		}
	
		Uint8 *pColorTab = (Uint8 *)mPixmap.colorTab;	
		for (Uint16 i=0; i<mPixmap.colorCount; i++)
		{
			*pColorTab++ = *(palette+3*i);     //Red
			*pColorTab++ = *(palette+3*i+1);   //Green
			*pColorTab++ = *(palette+3*i+2);   //Blue
			*pColorTab++ = 0;
		}
	}
	
	mLine = 0;
	mByte = 0;       
	mPass = 0;	      
	
	if (mLineBuffer)
	{
		UMemory::Dispose((TPtr)mLineBuffer);
		mLineBuffer = nil;
	}
}

// alloc memory
void CDecompressGif::AllocMemory()
{
	if (mPixmap.data)
		UMemory::Dispose((TPtr)mPixmap.data);
	
	try
	{
		mPixmap.data = UMemory::NewClear(mPixmap.rowBytes * mPixmap.height);
	}
	catch(...)
	{
		Fail(errorType_Image, imgError_InsufficientMemory);
	}
}

// this function is called when the GIF decoder encounters an extension 
bool CDecompressGif::PutExtension()
{
	PLAINTEXT pt;
	APPLICATION ap;
	Uint8 n;
	
extensionagain:

	switch (mExtensionStage) 
	{
		case 0x10:
			if (ReadImage(&mExtensionStage, 1) != END_OF_BYTES)
				goto extensionagain;
							
			break;

		case 0x11:
			if (ReadImage(&n, 1) != END_OF_BYTES)
			{
				if (n == 0)
				{
					mExtensionStage = 0x12; 
					goto extensionagain;
				}

				mOffset += n;			
				goto extensionagain;
			}
			break;

		case 0x12:
			return true;
		
		case 0x01:		
			// plain text descriptor 
			if (ReadImage(&pt, sizeof(PLAINTEXT)) != END_OF_BYTES)
			{
				#if !CONVERT_INTS
				pt.left = swap_int(pt.left);
				pt.top = swap_int(pt.top);
				pt.gridwidth = swap_int(pt.gridwidth);
				pt.gridheight = swap_int(pt.gridheight);
				#endif
					
				mExtensionStage = 0x11;
				goto extensionagain;
			}
			break;
						
		case 0xf9:		
			// graphic control block 
			if (ReadImage(&mCtrBlock, sizeof(CONTROLBLOCK)) != END_OF_BYTES)
			{
				#if !CONVERT_INTS
				mCtrBlock.delay = swap_int(mCtrBlock.delay);
				#endif

				mExtensionStage = 0x12; 
				goto extensionagain;
			}
			break;
			
		case 0xfe:		
			// comment extension 
			mExtensionStage = 0x11;
			goto extensionagain;
			break;
			
		case 0xff:		
			// application extension 
			if (ReadImage(&ap, sizeof(APPLICATION)) != END_OF_BYTES)
			{
				mExtensionStage = 0x11; 
				goto extensionagain;
			}
			break;
			
		default:		// something else 
			Uint32 nRet = ReadImage(&n, 1, true);
			if (nRet != END_OF_BYTES)
			{
				if(nRet != END_OF_FB)
					mOffset += n;

					
				mExtensionStage = 0x12; 
				goto extensionagain;	
			}
	};
	
	return false;
}

// unpack an LZW compressed image 
bool CDecompressGif::UnpackImage(Uint8 Bits1)
{
	static Uint16 wordmasktable[] = {	0x0000, 0x0001, 0x0003, 0x0007,
										0x000f, 0x001f, 0x003f, 0x007f,
										0x00ff, 0x01ff, 0x03ff, 0x07ff,
										0x0fff, 0x1fff, 0x3fff, 0x7fff
  								    };
 
	static Uint16 inctable[] = { 8, 8, 4, 2, 0 };   // interlace increments 
	static Uint16 startable[] = { 0, 4, 2, 1, 0 };  // interlace starts 

unpackagain:

	switch (mUnpackStage)
	{
		case 1:
			mpCurrentByte = mpLastByte = mpReadBuffer;
			mBitsLeft = 8;

			if (Bits1 < 2 || Bits1 > 8) 
				Fail(errorType_Image, imgError_BadSymbolSize);
		
			mBits2 = 1 << Bits1;
			mNextCode = mBits2 + 2;
			mCodeSize2 = 1 << (mCodeSize1 = Bits1 + 1);
			mOldCode = mOldToken = NO_CODE;

			try
			{
				mLineBuffer=(Uint8 *)UMemory::NewClear(mImageInfo.width);
			}
			catch(...)
			{
				Fail(errorType_Image, imgError_InsufficientMemory);
			}
			
			mUnpackStage = 2;
			goto unpackagain;
			break;
			
		case 2:
			if (mBitsLeft==8) 
			{
				if (++mpCurrentByte >= mpLastByte)
				{
					if (ReadImage(&mBlockSize, 1) != END_OF_BYTES)
					{
						// This is for fixing a GIF Library BUG
						if (mBlockSize == 0 && mLine == mImageInfo.depth && !(mImageInfo.flags & 0x40))
						{
							mOffset--;
					
							mUnpackStage = 11; 
							goto unpackagain;
						}

						mUnpackStage = 3;
						goto unpackagain;
					}
				}
				else
				{
					mBitsLeft = 0;
			
					mUnpackStage = 4;
					goto unpackagain;
				}
			}
			else
			{
				mUnpackStage = 4;
				goto unpackagain;
			}
			break;
			
		case 3:
			if (ReadImage(mpReadBuffer, mBlockSize) != END_OF_BYTES)
			{
				if (mBlockSize < 1 || (mpLastByte=(mpCurrentByte=mpReadBuffer)+mBlockSize) < (mpReadBuffer+mBlockSize)) 
					Fail(errorType_Image, imgError_InvalidFileStructure);
					
				mBitsLeft = 0;
				
				mUnpackStage = 4;
				goto unpackagain;				
			}
			break;
		
		case 4:
			mThisCode = *mpCurrentByte;
			if ((mCurrentCode=(mCodeSize1 + mBitsLeft)) <= 8) 
			{
				*mpCurrentByte >>= mCodeSize1;
				mBitsLeft = mCurrentCode;
				
				mUnpackStage = 10; 
				goto unpackagain;				
			}
			else 
			{
				mUnpackStage = 5; 
				goto unpackagain;				
			}
			break;
			
		case 5:
			if (++mpCurrentByte >= mpLastByte)
			{
				if (ReadImage(&mBlockSize, 1) != END_OF_BYTES)
				{
					// This is for fixing a GIF Library BUG
					if (mBlockSize == 0 && mLine == mImageInfo.depth && !(mImageInfo.flags & 0x40))
					{
						mOffset--;
					
						mUnpackStage = 11; 
						goto unpackagain;
					}
			
					mUnpackStage = 6;
					goto unpackagain;
				}
			}
			else
			{
				mUnpackStage = 7;
				goto unpackagain;
			}
			break;			
			
		case 6:
			if (ReadImage(mpReadBuffer, mBlockSize) != END_OF_BYTES)
			{
				if (mBlockSize < 1 || (mpLastByte=(mpCurrentByte=mpReadBuffer)+mBlockSize) < (mpReadBuffer+mBlockSize)) 
					Fail(errorType_Image, imgError_InvalidFileStructure);
				
				mUnpackStage = 7;
				goto unpackagain;
			}
			break;
			
		case 7:
			mThisCode |= *mpCurrentByte << (8 - mBitsLeft);
			if (mCurrentCode <= 16) 
			{
				*mpCurrentByte >>= (mBitsLeft = mCurrentCode-8);
				
				mUnpackStage = 10;  
				goto unpackagain;						
			}
			else 
			{
				mUnpackStage = 8;
				goto unpackagain;
			}
			break;
			
		case 8:
			if (++mpCurrentByte >= mpLastByte)
			{
				if (ReadImage(&mBlockSize, 1) != END_OF_BYTES)
				{
					// This is for fixing a GIF Library BUG
					if (mBlockSize == 0 && mLine == mImageInfo.depth && !(mImageInfo.flags & 0x40))
					{
						mOffset--;
					
						mUnpackStage = 11; 
						goto unpackagain;
					}

					mUnpackStage = 9;
					goto unpackagain;
				}
			}
			else
			{
				mThisCode |= *mpCurrentByte << (16 - mBitsLeft);
				*mpCurrentByte >>= (mBitsLeft = mCurrentCode - 16);

				mUnpackStage = 10;
				goto unpackagain;
			}
			break;
			
		case 9:
			if (ReadImage(mpReadBuffer, mBlockSize) != END_OF_BYTES)
			{
				if (mBlockSize < 1 || (mpLastByte=(mpCurrentByte=mpReadBuffer)+mBlockSize) < (mpReadBuffer+mBlockSize)) 
					Fail(errorType_Image, imgError_InvalidFileStructure);
				
				mThisCode |= *mpCurrentByte << (16 - mBitsLeft);
				*mpCurrentByte >>= (mBitsLeft = mCurrentCode - 16);
				
				mUnpackStage = 10;
				goto unpackagain;
			}
			break;
			
		case 10:
			mThisCode &= wordmasktable[mCodeSize1];
			mCurrentCode = mThisCode;

			if (mThisCode == (mBits2+1))  // found EOI 
			{
				mUnpackStage = 11;
				goto unpackagain;
			}
			
			if (mThisCode > mNextCode) 
				Fail(errorType_Image, imgError_InvalidFileStructure);

			if (mThisCode == mBits2) 
			{
				mNextCode = mBits2 + 2;
				mCodeSize2 = 1 << (mCodeSize1 = (Bits1 + 1));
				mOldToken = mOldCode = NO_CODE;
	
				mUnpackStage = 2;
				goto unpackagain;
			}

			mpStack = mpFirstCodeStack;

			if (mThisCode==mNextCode) 
			{
				if (mOldCode==NO_CODE) 
					Fail(errorType_Image, imgError_InvalidFileStructure);
			
				*mpStack++ = mOldToken;
				mThisCode = mOldCode;
			}

			while (mThisCode >= mBits2) 
			{
				*mpStack++ = mpLastCodeStack[mThisCode];
				mThisCode = mpCodeStack[mThisCode];
			}

			mOldToken = mThisCode;
			do 
			{
				mLineBuffer[mByte++] = mThisCode;
				if (mByte >= mImageInfo.width) 
				{
					PutLine(mLineBuffer, mLine);
					mByte=0;

					// check for interlaced image 
					if (mImageInfo.flags & 0x40) 
					{
						mLine+=inctable[mPass];
						if (mLine >= mImageInfo.depth)
							 mLine=startable[++mPass];
					} 
					else 
						++mLine;
				}

				if (mpStack <= mpFirstCodeStack) 
					break;				
					
				mThisCode = *--mpStack;
			} while(1);

			if (mNextCode < 4096 && mOldCode != NO_CODE) 
			{
				mpCodeStack[mNextCode] = mOldCode;
				mpLastCodeStack[mNextCode] = mOldToken;
				if (++mNextCode >= mCodeSize2 && mCodeSize1 < 12)
					 mCodeSize2 = 1 << ++mCodeSize1;
			}
		
			mOldCode = mCurrentCode;
	
			mUnpackStage = 2;
			goto unpackagain;
			break;
		
		case 11:
			UMemory::Dispose((TPtr)mLineBuffer);
			mLineBuffer = nil;
			
			mUnpackStage = 12;
			goto unpackagain;
			break;

		case 12:
			return true;
	};

	return false;
}

// save one line to memory 
void CDecompressGif::PutLine(Uint8 *pLine, Uint16 nLine)
{
	if (nLine >= mImageInfo.depth)
		return;
  	
	Uint32 nPoz=(Uint32)nLine*mImageInfo.width;
	UMemory::Copy((Uint8*)mPixmap.data+nPoz, pLine, mImageInfo.width);

	mReadRows = nLine+1; 
	if (mImageInfo.flags & 0x40)  // interlaced image
	{
		Uint8 nFillLines = 0;
		if (!(nLine%8))
			nFillLines = 8;
		else if (!(nLine%4))
			nFillLines = 4;
		else if (!(nLine%2))
			nFillLines = 2;
		else if (nLine >= mImageInfo.depth-2)
		{
			mReadRows = mImageInfo.depth;
			
			mFullUpdate = true;
		}

		if (nFillLines)
		{
			Uint16 i;
			for (i=nLine+1; i<nLine+nFillLines && i<mImageInfo.depth; i++)
			{
				nPoz=(Uint32)i*mImageInfo.width;
				UMemory::Copy((Uint8*)mPixmap.data+nPoz, pLine, mImageInfo.width);
			}
		
			mReadRows = i;
		}
	}
}

