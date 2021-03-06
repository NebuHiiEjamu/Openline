/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "CSimpleIconButtonView.h"

#if WIN32
HDC _ImageToDC(TImage inImage);
#endif

#pragma options align=packed
typedef struct {
	Int16 enabledIcon, disabledIcon, hilitedIcon;
} SSimpleIconButtonView;
#pragma options align=reset

/* -------------------------------------------------------------------------- */

CSimpleIconButtonView::CSimpleIconButtonView(CViewHandler *inHandler, const SRect& inBounds)
	: CView(inHandler, inBounds)
{
#if WIN32
	mCanFocus = true;
#endif

	mEnabledIcon = mDisabledIcon = mHilitedIcon = nil;
	mIsFocused = mIsHilited = mIsDrawDisabled = false;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CSimpleIconButtonView::SetIcons(TIcon inEnabled, TIcon inDisabled, TIcon inHilited)
{
	UIcon::Release(mEnabledIcon);
	UIcon::Release(mDisabledIcon);
	UIcon::Release(mHilitedIcon);
	
	mEnabledIcon = inEnabled;
	mDisabledIcon = inDisabled;
	mHilitedIcon = inHilited;
	
	Refresh();
}

void CSimpleIconButtonView::SetIconIDs(Int32 inEnabledID, Int32 inDisabledID, Int32 inHilitedID)
{
	SetIcons(UIcon::Load(inEnabledID), UIcon::Load(inDisabledID), UIcon::Load(inHilitedID));
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CSimpleIconButtonView::Draw(TImage inImage, const SRect& /* inUpdateRect */, Uint32 /* inDepth */)
{
	TIcon icon = mEnabledIcon;
	Int16 trans = transform_None;

	if (mIsDrawDisabled)
	{
		if (mDisabledIcon)
			icon = mDisabledIcon;
		else
			trans = transform_Light;
	}
	else if (mIsHilited)
	{
		if (mHilitedIcon)
			icon = mHilitedIcon;
		else
			trans = transform_Dark;
	}

	icon->Draw(inImage, mBounds, align_Center, trans);
	
#if WIN32
	if (mIsFocused)
	{
		RECT r = *(RECT *)&mBounds;
		HDC dc = _ImageToDC(inImage);
	
		DrawFocusRect(dc, &r);
	}
#endif
}

void CSimpleIconButtonView::MouseDown(const SMouseMsgData& inInfo)
{
	inherited::MouseDown(inInfo);
	
	if (IsEnabled() && IsActive())
	{
		mIsHilited = true;
		Refresh();
	}
}

void CSimpleIconButtonView::MouseUp(const SMouseMsgData& inInfo)
{
	inherited::MouseUp(inInfo);
	
	if (mIsHilited)
	{
		mIsHilited = false;
		Refresh();

		if (IsMouseWithin())
			Hit((inInfo.mods & modKey_Option) ? hitType_Alternate : hitType_Standard, inInfo.button, inInfo.mods);
	}
}

void CSimpleIconButtonView::MouseEnter(const SMouseMsgData& inInfo)
{
	inherited::MouseEnter(inInfo);
	
	if (!mIsHilited && IsAnyMouseBtnDown() && IsEnabled() && IsActive())
	{
		mIsHilited = true;
		Refresh();
	}
}

void CSimpleIconButtonView::MouseLeave(const SMouseMsgData& inInfo)
{
	inherited::MouseLeave(inInfo);
	
	if (mIsHilited)
	{
		mIsHilited = false;
		Refresh();
	}
}

bool CSimpleIconButtonView::KeyDown(const SKeyMsgData& inInfo)
{
	if (IsEnabled() && IsActive())
	{
		if (inInfo.keyCode == key_nEnter || inInfo.keyCode == key_Return || inInfo.keyCode == key_Escape || (inInfo.keyChar == '.' && (inInfo.mods & modKey_Command)))
		{
			Hit((inInfo.mods & modKey_Option) ? hitType_Alternate : hitType_Standard);
			
			return true;
		}
	}
	
	return false;
}

void CSimpleIconButtonView::UpdateActive()
{
	bool bStateChanged = false;
	
	bool bIsFocused = IsEnabled() && IsFocus();
	if (bIsFocused != mIsFocused)
	{
		mIsFocused = bIsFocused;
		if (mIsFocused) mIsDrawDisabled = mIsHilited = false;
		bStateChanged = true;
	}

	bool bDrawDisab = IsDisabled() || IsInactive();
	if (bDrawDisab != mIsDrawDisabled)	// if change
	{
		mIsDrawDisabled = bDrawDisab;
		if (mIsDrawDisabled) mIsHilited = false;
		bStateChanged = true;
	}
	
	if (bStateChanged)
		Refresh();
}

bool CSimpleIconButtonView::ChangeState(Uint16 inState)
{
	if (mState != inState)	// if change
	{
		mState = inState;
		UpdateActive();
		return true;
	}
	
	return false;
}

bool CSimpleIconButtonView::SetEnable(bool inEnable)
{
	if (mIsEnabled != (inEnable != 0))		// if change
	{
		mIsEnabled = (inEnable != 0);
		UpdateActive();
		
		return true;
	}
	
	return false;
}


