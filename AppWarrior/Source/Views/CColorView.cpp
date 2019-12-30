/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "CColorView.h"

/* -------------------------------------------------------------------------- */

CColorView::CColorView(CViewHandler *inHandler, const SRect& inBounds)
	: CView(inHandler, inBounds)
{
#if WIN32
	mCanFocus = true;
#endif

	mColor.red = mColor.green = mColor.blue = 0xFFFF;
	mIsFocused = mIsHilited = false;
}

CColorView::CColorView(CViewHandler *inHandler, const SRect& inBounds, const SColor& inColor)
	: CView(inHandler, inBounds)
{
#if WIN32
	mCanFocus = true;
#endif

	mColor = inColor;
	mIsFocused = mIsHilited = false;
}

void CColorView::SetColor(const SColor& inColor)
{
	if (mColor != inColor)
	{
		mColor = inColor;
		Refresh();
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CColorView::Draw(TImage inImage, const SRect& /* inUpdateRect */, Uint32 /* inDepth */)
{
	SRect r = mBounds;
	
	UUserInterface::DrawStandardBox(inImage, mBounds, nil, !mIsEnabled, false, false);
	
	// draw white part of content
	r.Enlarge(-2, -2);
	inImage->SetPenSize(2);
	inImage->SetInkColor(color_White);
	inImage->FrameRect(r);
	inImage->SetPenSize(1);
	
	// draw color
	r.Enlarge(-2, -2);
	inImage->SetInkColor(mColor);
	inImage->FillRect(r);
	
	// draw hilite
	if (mIsFocused || mIsHilited)
	{
		r = mBounds;
		r.Enlarge(-2, -2);
		inImage->SetInkColor(color_Black);
		inImage->FrameRect(r);
	}
}

void CColorView::MouseDown(const SMouseMsgData& inInfo)
{
	inherited::MouseDown(inInfo);
	
	if (IsEnabled() && IsActive())
	{
		mIsHilited = true;
		Refresh();
	}
}

void CColorView::MouseUp(const SMouseMsgData& inInfo)
{
	inherited::MouseUp(inInfo);
	
	if (mIsHilited)
	{
		mIsHilited = false;
		Refresh();
		
		if (IsMouseWithin())
		{
			if (UGraphics::UserSelectColor(mColor))
			{
				Refresh();
				Hit();
			}
		}
	}
}

void CColorView::MouseEnter(const SMouseMsgData& inInfo)
{
	inherited::MouseEnter(inInfo);
	
	if (!mIsHilited && IsAnyMouseBtnDown() && IsEnabled() && IsActive())
	{
		mIsHilited = true;
		Refresh();
	}
}

void CColorView::MouseLeave(const SMouseMsgData& inInfo)
{
	inherited::MouseLeave(inInfo);
	
	if (mIsHilited)
	{
		mIsHilited = false;
		Refresh();
	}
}

bool CColorView::KeyDown(const SKeyMsgData& inInfo)
{
	if (IsEnabled() && IsActive() && (inInfo.keyCode == key_nEnter || inInfo.keyCode == key_Return))
	{
		if (UGraphics::UserSelectColor(mColor))
		{
			Refresh();
			Hit();
		}
	
		return true;
	}
	
	return false;
}

void CColorView::UpdateActive()
{
	bool bStateChanged = false;
	
	bool bIsFocused = IsEnabled() && IsFocus();
	if (bIsFocused != mIsFocused)
	{
		mIsFocused = bIsFocused;
		if (mIsFocused) mIsHilited = false;
		bStateChanged = true;
	}

	if (mIsHilited && (IsDisabled() || IsInactive()))
	{
		mIsHilited = false;
		bStateChanged = true;
	}
	
	if (bStateChanged)
		Refresh();
}

bool CColorView::ChangeState(Uint16 inState)
{
	if (mState != inState)	// if change
	{
		mState = inState;
		UpdateActive();
		return true;
	}
	
	return false;
}

bool CColorView::SetEnable(bool inEnable)
{
	if (mIsEnabled != (inEnable != 0))		// if change
	{
		mIsEnabled = (inEnable != 0);
		mIsHilited = false;
		Refresh();
		
		return true;
	}
	
	return false;
}


