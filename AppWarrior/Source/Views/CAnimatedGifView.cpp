/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "CAnimatedGifView.h"


CAnimatedGifView::CAnimatedGifView(CViewHandler *inHandler, const SRect& inBounds, CDecompressGif *inDecompressGif)
		: CImageView(inHandler, inBounds, inDecompressGif)
{
	ClearStruct(mGifInfo);

	mImageCount = 0;
	mAnimatedGif = false;

	mCurrentBackImage = nil;
	mNewBackImage = nil;
	mPrevBackImage = nil;
	mBackImage = false;

	mBackColor = false;
	mImageTimer = nil;	
	mStartTimer = false;
}

CAnimatedGifView::~CAnimatedGifView()
{
	Uint32 i = 0;
	SImageInfo *pi;

	while (mImageInfoList.GetNext(pi, i))
	{
		if (pi->image)
			UGraphics::DisposeImage(pi->image);
			
		if (pi->mask)
			UGraphics::DisposeImage(pi->mask);
			
		UMemory::Dispose((TPtr)pi);
	}
	
	mImageInfoList.Clear();
		
	if (mCurrentBackImage)
		UGraphics::DisposeImage(mCurrentBackImage);

	if (mNewBackImage)
		UGraphics::DisposeImage(mNewBackImage);

	if (mPrevBackImage)
		UGraphics::DisposeImage(mPrevBackImage);
	
	if (mImageTimer)
		UTimer::Dispose(mImageTimer);
}

void CAnimatedGifView::ImageTimer(void *inContext, void */* inObject */, Uint32 /* inMsg */, const void */* inData */, Uint32 /* inDataSize */)
{
	CAnimatedGifView *gifView = (CAnimatedGifView *)inContext;

	if (!gifView->mStartTimer)
		return;
	
	gifView->RestoreBackground();
	
	gifView->mImageCount++;
	if (gifView->mImageCount > gifView->mImageInfoList.GetItemCount())
		gifView->mImageCount = 1;

	gifView->SaveBackground();
	gifView->CheckTimer();	
	gifView->Refresh();
}

void CAnimatedGifView::CheckTimer()
{
	if (!mImageTimer)
		return;
	
	if (!(mImageInfoList.GetItem(mImageCount)->ctrBlock.flags & 0x02))
	{
		Uint32 nDelay = mImageInfoList.GetItem(mImageCount)->ctrBlock.delay;

		if(nDelay > 0)
			nDelay *= 10;   
		else
			nDelay = 80;  
	
		mImageTimer->Start(nDelay);
		
		if(!mStartTimer)
			mStartTimer = true;
	}
	else if (mStartTimer)
	{
		mStartTimer = false;
	}
}

void CAnimatedGifView::SaveBackground()
{
	SRect rImageRect(0, 0, mGifInfo.width, mGifInfo.depth);
		
	Int8 nFlags = mImageInfoList.GetItem(mImageCount)->ctrBlock.flags;
	if (((nFlags & 0x1C) == 0x04) || ((nFlags & 0x1C) == 0x00))              // Do not dispose
	{
    	if (!mNewBackImage)
			InitCompatibleImage(mNewBackImage);
	    	
	    StImageLocker lockImage(mNewBackImage);
		DrawImage(mNewBackImage, rImageRect, rImageRect, mImageCount, mCurrentBackImage);
	}
	
	Uint16 nCount =	mImageCount + 1;
	if (nCount > mImageInfoList.GetItemCount())
		nCount = 1;
			
	if ((mImageInfoList.GetItem(nCount)->ctrBlock.flags & 0x1C) == 0x0C)       // Restore to previous
	{
    	if (!mPrevBackImage)
			InitCompatibleImage(mPrevBackImage);
	    	
	    StImageLocker lockImage(mPrevBackImage);
		DrawImage(mPrevBackImage, rImageRect, rImageRect, mImageCount, mCurrentBackImage);
	}
}

void CAnimatedGifView::RestoreBackground()
{
	Int8 nFlags = mImageInfoList.GetItem(mImageCount)->ctrBlock.flags;
	
	mBackImage = false;
	if ((((nFlags & 0x1C) == 0x04) || ((nFlags & 0x1C) == 0x00)) && mNewBackImage) // Do not dispose
	{
	   	if (!mCurrentBackImage)
			InitCompatibleImage(mCurrentBackImage);
	    	
	    StImageLocker lockCurrentImage(mCurrentBackImage);
  	    StImageLocker lockNewImage(mNewBackImage);
	   	UGraphics::CopyPixels(mCurrentBackImage, SPoint(0, 0), mNewBackImage, SPoint(0, 0), mGifInfo.width, mGifInfo.depth);
	 	mBackImage = true;  	
	}
	
	mBackColor = false;
	if (((nFlags & 0x1C) == 0x08) && (mGifInfo.flags & 0x80))    // Restore to background color
		mBackColor = true;

	if (((nFlags & 0x1C) == 0x0C) && mPrevBackImage)             // Restore to previous
	{
	   	if(!mCurrentBackImage)
			InitCompatibleImage(mCurrentBackImage);
	    	
	    StImageLocker lockCurrentImage(mCurrentBackImage);
  	    StImageLocker lockPrevImage(mPrevBackImage);
	   	UGraphics::CopyPixels(mCurrentBackImage, SPoint(0, 0), mPrevBackImage, SPoint(0, 0), mGifInfo.width, mGifInfo.depth);
	 	mBackImage = true;  	
	}
}

void CAnimatedGifView::Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth)
{
	if (inUpdateRect.IsEmpty())
		return;

	if (!mAnimatedGif)
	{
		CImageView::Draw(inImage, inUpdateRect, inDepth);
		return;
	}

	SRect rViewRect;
	GetBounds(rViewRect);
	if (!DrawImage(inImage, inUpdateRect, rViewRect, mImageCount, mCurrentBackImage))
		inImage->Reset();
}

bool CAnimatedGifView::DrawImage(TImage outImage, const SRect& inUpdateRect, const SRect& inViewRect, Uint16 inImageCount, TImage inBackImage)
{
	if (mBackImage && !DrawBackImage(outImage, inUpdateRect, inViewRect, inBackImage))
		return false;

	if (mBackColor)
	{
		Uint8 *palette = mGifInfo.palette + 3*mGifInfo.background;
		outImage->SetInkColor(SColor(*(palette) * 256, *(palette+1) * 256, *(palette+2) * 256));
		outImage->FillRect(inUpdateRect);
	}
	
	if (!DrawForeImage(outImage, inUpdateRect, inViewRect, inImageCount))
		return false;
	
	return true;
}

bool CAnimatedGifView::DrawBackImage(TImage outImage, const SRect& inUpdateRect, const SRect& inViewRect, TImage inBackImage)
{
    if (!inBackImage)
    	return false;
    	
	SPoint ptImage(inViewRect.left, inViewRect.top); 
	SRect rImage(ptImage.x, ptImage.y, ptImage.x + mGifInfo.width, ptImage.y + mGifInfo.depth);

    SRect rUpdateRect;
    inUpdateRect.GetIntersection(rImage, rUpdateRect);
		    
	if (rUpdateRect.IsNotEmpty() && rUpdateRect.IsValid())
	{
	    StImageLocker lockImage(inBackImage);
    	UGraphics::CopyPixels(outImage, rUpdateRect.TopLeft() , inBackImage, SPoint(rUpdateRect.left-ptImage.x, rUpdateRect.top-ptImage.y), rUpdateRect.GetWidth(), rUpdateRect.GetHeight());
	}
	
	return true;
}

bool CAnimatedGifView::DrawForeImage(TImage outImage, const SRect& inUpdateRect, const SRect& inViewRect, Uint16 inImageCount)
{
	SImageInfo *pi = mImageInfoList.GetItem(inImageCount);
    if (!pi->image)
    	return false;
    	
	SPoint ptImage(inViewRect.left + pi->imgBlock.left, inViewRect.top + pi->imgBlock.top); 
	SRect rImage(ptImage.x, ptImage.y, ptImage.x + pi->imgBlock.width, ptImage.y + pi->imgBlock.depth);

    SRect rUpdateRect;
    inUpdateRect.GetIntersection(rImage, rUpdateRect);
		    
	if (rUpdateRect.IsNotEmpty() && rUpdateRect.IsValid())
	{
	    StImageLocker lockImage(pi->image);
	    if ((pi->ctrBlock.flags & 0x01) && pi->mask)
	    {
			StImageLocker lockMask(pi->mask);
			UGraphics::CopyPixelsMasked(outImage, rUpdateRect.TopLeft(), pi->image, SPoint(rUpdateRect.left-ptImage.x, rUpdateRect.top-ptImage.y), rUpdateRect.GetWidth(), rUpdateRect.GetHeight(), pi->mask);
		}
	    else
	    {
	    	UGraphics::CopyPixels(outImage, rUpdateRect.TopLeft(), pi->image, SPoint(rUpdateRect.left-ptImage.x, rUpdateRect.top-ptImage.y), rUpdateRect.GetWidth(), rUpdateRect.GetHeight());
	    }
	}
	
	return true;
}

void CAnimatedGifView::MouseUp(const SMouseMsgData& inInfo)
{
	if (mImageTimer && inInfo.button == mouseButton_Left && !mStartTimer && (mImageInfoList.GetItem(mImageCount)->ctrBlock.flags & 0x02))
	{
		mImageTimer->Start(0);
		mStartTimer = true;
	}

	inherited::MouseUp(inInfo);
}

void CAnimatedGifView::KeyUp(const SKeyMsgData& inInfo)
{
	if (mImageTimer && inInfo.keyCode == key_Space)
	{
		if (mStartTimer)
			StopImage();
		else
			PlayImage();
	}
	
	inherited::KeyUp(inInfo);
}

void CAnimatedGifView::PlayImage()
{
	if (!mStartTimer)
	{
		mImageTimer->Start(0);
		mStartTimer = true;
	}
}

void CAnimatedGifView::StopImage()
{
	if (mStartTimer)
	{
		mStartTimer = false;
		mImageTimer->Stop();
	}
}

bool CAnimatedGifView::SetImage(Uint32 inWidth, Uint32 inHeight)
{
	if (!mAnimatedGif && mImageCount == 1)		
		mAnimatedGif = true; 	

	if (mDecompressImage)
	{
		if(!mGifInfo.width || !mGifInfo.depth)
		{
			mGifInfo = ((CDecompressGif*)mDecompressImage)->GetGifInfo();
			mWidth = mGifInfo.width;
			mHeight = mGifInfo.depth;
		}
		
		if(((CDecompressGif*)mDecompressImage)->IsCompleteImage())
			PutImageInList();
	}
		
	if (!inWidth || !inHeight)
		return CImageView::SetImage(mGifInfo.width, mGifInfo.depth);
	else
		return CImageView::SetImage(inWidth, inHeight);
}

bool CAnimatedGifView::FinishImage()
{
	InitAnimatedGif();
	mDecompressImage = nil;
	
	mIsComplete = true;
	
	return true;
}

void CAnimatedGifView::InitAnimatedGif()
{
	Uint32 nItemCount = mImageInfoList.GetItemCount();
	if (nItemCount > 1)
	{
		mImageCount = 1;		

		SaveBackground();
		if (((mImageInfoList.GetItem(mImageInfoList.GetItemCount())->ctrBlock.flags & 0x1C) == 0x08) && (mGifInfo.flags & 0x80))
			mBackColor = true;

		if (mImageTimer == nil)
			mImageTimer = UTimer::New(ImageTimer, this);
		
		CheckTimer();
	} 
	else if (nItemCount == 1)
	{
		mImageCount = 0;

		SImageInfo *pi = mImageInfoList.GetItem(nItemCount);
		mImage = pi->image;

		if (pi->mask)
			UGraphics::DisposeImage(pi->mask);

		UMemory::Dispose((TPtr)pi);
		mImageInfoList.Clear();
	}
}

void CAnimatedGifView::PutImageInList()
{
	if (mDecompressImage)
	{
		SImageInfo *pi = (SImageInfo *)UMemory::NewClear(sizeof(SImageInfo));
		
		pi->image = mDecompressImage->DetachImagePtr();
		pi->mask = ((CDecompressGif*)mDecompressImage)->GetTransMask();
		pi->imgBlock = ((CDecompressGif*)mDecompressImage)->GetImgBlock();
		pi->ctrBlock = ((CDecompressGif*)mDecompressImage)->GetCtrBlock();

		mImageInfoList.AddItem(pi);

		if (IsVisible() && mImageCount == 1)
			SaveBackground();
	
		if (IsVisible() && mImageCount >= 1)
			RestoreBackground(); 

		mImageCount++;				
				
		if (IsVisible() && mImageCount >= 2)
			SaveBackground(); 
	}
}

bool CAnimatedGifView::InitCompatibleImage(TImage &outImage)
{
	if (outImage)
		return false;
	
	outImage = UGraphics::NewCompatibleImage(mGifInfo.width, mGifInfo.depth);
	if (!outImage)
		Fail(errorType_Image, imgError_InsufficientMemory);

	outImage->SetInkColor(SColor(65535, 65535, 65535));
	
	StImageLocker lockImage(outImage);
	outImage->FillRect(SRect(0, 0, mGifInfo.width, mGifInfo.depth));
	
	return true;
}
