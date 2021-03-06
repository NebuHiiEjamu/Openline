/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "CIconView.h"

#pragma options align=packed

typedef struct {
	Int16 iconID;
} SIconView;

#pragma options align=reset

/* -------------------------------------------------------------------------- */

CIconView::CIconView(CViewHandler *inHandler, const SRect& inBounds)
	: CView(inHandler, inBounds)
{
	mIcon = nil;
}

CIconView::CIconView(CViewHandler *inHandler, const SRect& inBounds, Int32 inIconID)
	: CView(inHandler, inBounds)
{
	mIcon = UIcon::Load(inIconID);
}

void CIconView::SetIconID(Int32 inID)
{
	SetIcon(UIcon::Load(inID));
}

// takes ownership of the icon
void CIconView::SetIcon(TIcon inIcon)
{
	if (inIcon == mIcon)
	{
		UIcon::Release(inIcon);
	}
	else
	{
		UIcon::Release(mIcon);
		mIcon = inIcon;
		Refresh();
	}
}

TIcon CIconView::GetIcon() const
{
	return mIcon;
}

void CIconView::Draw(TImage inImage, const SRect& /* inUpdateRect */, Uint32/* inDepth */)
{
	mIcon->Draw(inImage, mBounds, align_Center, transform_None);
}




