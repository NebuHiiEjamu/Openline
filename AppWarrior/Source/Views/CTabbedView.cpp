/****************************************

Extract this out into UTabs

- includes funcs for drawing
  (based on generic data ptrs etc - don't force format)

- includes funcs for storing tab info (in mem) but don't HAVE to use!
Also funcs for flattening/unflattening.

**********************************************/



#include "CTabbedView.h"
#include "UMemory.h"

/*
 * Structures
 */

// begin structures that must be the same across platforms
#pragma options align=packed

typedef struct {
	Int16 textStyle;
	Int16 textScript;
	Uint16 tabHeight;
	Uint16 extraTabWidth;
	Uint16 currentTab;
	Uint32 rsvd[4];
	Uint16 tabCount;
	Uint8 data[];
	
	/* Data is formatted like this:
	 *
	 * 		struct {
	 *			Uint32 viewID;
	 *			Uint8 titleLen;
	 *			Uint8 titleData[titleLen];
	 *		} tab[tabCount];
	 */
} STabbedView;

// end structures that must be the same across platforms
#pragma options align=reset

struct STabInfo {
	CView *view;
	THdl title;
	Int16 width;
};

/* -------------------------------------------------------------------------- */

CTabbedView::CTabbedView(CViewHandler *inHandler, const SRect& inBounds)
	: CView(inHandler, inBounds)
{
	mTabInfo = nil;
	mFont = nil;
	mTextScript = 0;
	mTabCount = 0;
	mCurrentTab = 0;
	mTabHeight = 20;
	mHilitedTab = 0;
	mMouseCaptureTab = 0;
	mMouseCaptureView = nil;
	mCanFocus = true;		// tabbed view can select by default
	//mDragWithinViewCanAccept = 
	mMouseDownInTabs = mIsDrawDisabled = false;
}

CTabbedView::~CTabbedView()
{
	delete mFont;
	if (mTabInfo)
	{
		STabInfo *tabInfo = (STabInfo *)UMemory::Lock(mTabInfo);
		
		for (Uint32 i=0; i<mTabCount; i++)
		{
			UMemory::Dispose((THdl)tabInfo[i].title);
		}
		
		UMemory::Unlock(mTabInfo);
		UMemory::Dispose(mTabInfo);
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

Uint16 CTabbedView::GetTabCount() const
{
	return mTabCount;
}

void CTabbedView::InsertTab(Uint16 inBeforeTab, const void *inText, Uint32 inSize)
{
	STabInfo tabInfo;
	
	tabInfo.view = nil;
	tabInfo.title = UMemory::NewHandle(inText, inSize);
	tabInfo.width = GetTabWidth(inText, inSize);
	
	try
	{
		if (mTabCount == 0)
		{
			mTabInfo = UMemory::NewHandle(&tabInfo, sizeof(tabInfo));
			mTabCount = 1;
			inBeforeTab = 1;
		}
		else
		{
			if (inBeforeTab < 1)
				inBeforeTab = 0;
			else if (inBeforeTab > mTabCount)
				inBeforeTab = mTabCount + 1;
			
			UMemory::Insert(mTabInfo, (inBeforeTab - 1) * sizeof(STabInfo), &tabInfo, sizeof(STabInfo));
			mTabCount++;
		}
	}
	catch(...)
	{
		UMemory::Dispose(tabInfo.title);
		throw;
	}
	
	RefreshTabs(inBeforeTab, max_Uint16);
}

void CTabbedView::AddTab(const Uint8 inTitle[])
{
	InsertTab(max_Uint16, inTitle+1, inTitle[0]);
}

void CTabbedView::RemoveTab(Uint16 inTab)
{
	Require(IsValidTab(inTab));
	UMemory::Remove(mTabInfo, (inTab - 1) * sizeof(STabInfo), sizeof(STabInfo));
	mTabCount--;
		
	RefreshTabs(inTab, max_Uint16);
}

bool CTabbedView::IsValidTab(Uint16 inTab) const
{
	return (inTab && inTab <= mTabCount);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CTabbedView::SetTabTitle(Uint16 inTab, const void *inText, Uint32 inSize)
{
	Require(IsValidTab(inTab));
	
	STabInfo *tabInfo;
	StHandleLocker lock(mTabInfo, (void*&)tabInfo);
	
	THdl h = tabInfo[inTab-1].title;
	
	if (h == nil)
		tabInfo[inTab-1].title = UMemory::NewHandle(inText, inSize);
	else
		UMemory::Set(h, inText, inSize);
	
	RefreshTabs(inTab, max_Uint16);
}

void CTabbedView::SetTabTitle(Uint16 inTab, const Uint8 inText[])
{
	if (inText) SetTabTitle(inTab, inText+1, inText[0]);
}

Uint32 CTabbedView::GetTabTitle(Uint16 inTab, void *outText, Uint32 inMaxSize) const
{
	Require(IsValidTab(inTab));
	
	STabInfo *tabInfo;
	StHandleLocker lock(mTabInfo, (void*&)tabInfo);

	return UMemory::Read(tabInfo[inTab-1].title, 0, outText, inMaxSize);
}

// faster to set this BEFORE adding tabs
void CTabbedView::SetFont(TFontDesc inFont)
{
	delete mFont;
	mFont = inFont;
	
	
	// **** recalc widths
	
	// refresh all tabs
}

void CTabbedView::SetTabView(Uint16 inTab, CView *inView)
{
	Require(IsValidTab(inTab));
	
	STabInfo *tabInfo;
	StHandleLocker lock(mTabInfo, (void*&)tabInfo);

	CView *v = tabInfo[inTab-1].view;
	
	if (v != inView)
	{
		if (v)
		{
			tabInfo[inTab-1].view = nil;
			try { delete v; } catch(...) {}
		}
		
		if (inView)
		{
			inView->SetHandler(this);
			tabInfo[inTab-1].view = inView;
			inView->SetVisible(inTab == mCurrentTab);
		}
		
		if (inTab == mCurrentTab)
			RefreshContent();
	}
}

CView *CTabbedView::GetTabView(Uint16 inTab) const
{
	Require(IsValidTab(inTab));
	
	STabInfo *tabInfo;
	StHandleLocker lock(mTabInfo, (void*&)tabInfo);
	
	return tabInfo[inTab-1].view;
}

CView *CTabbedView::DetachTabView(Uint16 inTab)
{
	Require(IsValidTab(inTab));
	
	STabInfo *tabInfo;
	StHandleLocker lock(mTabInfo, (void*&)tabInfo);

	CView *v = tabInfo[inTab-1].view;
	
	if (v)
	{
		tabInfo[inTab-1].view = nil;
	
		v->SetHandler(nil);
		
		if (inTab == mCurrentTab)
			RefreshContent();
	}
	
	return v;
}

void CTabbedView::SetTabHeight(Uint16 inHeight)
{
	if (mTabHeight != inHeight)
	{
		mTabHeight = inHeight;
		
		// *** need to adjust the bounds of the views
		
		Refresh();
	}
}

Uint16 CTabbedView::GetTabHeight() const
{
	return mTabHeight;
}

void CTabbedView::SetCurrentTab(Uint16 inTab)
{
	if (mCurrentTab != inTab)	// if change
	{
		Require(IsValidTab(inTab));
		CView *v;
		
		if (mCurrentTab)
		{
			v = GetTabView(mCurrentTab);
			if (v) v->ChangeState(viewState_Hidden);
		}
		
		mCurrentTab = inTab;
		
		v = GetTabView(inTab);
		if (v)
		{
			v->ChangeState(mState);
			v->Show();		// just in case was invisible, as is when created
		}
		
		Refresh();
	}
}

Uint16 CTabbedView::GetCurrentTab() const
{
	return mCurrentTab;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CTabbedView::GetContentRect(SRect& outRect) const
{
	outRect.top = mBounds.top + mTabHeight + 3;
	outRect.left = mBounds.left + 3;
	outRect.bottom = mBounds.bottom - 3;
	outRect.right = mBounds.right - 3;
}

void CTabbedView::RefreshContent()
{
	SRect r;
	GetContentRect(r);
	Refresh(r);
}

void CTabbedView::RefreshTabs(Uint16 inStartTab, Uint16 inEndTab)
{
	SRect r;
	
	r.top = mBounds.top;
	r.bottom = r.top + mTabHeight;
	
	if (inStartTab < 1)
		r.left = mBounds.left;
	else
		r.left = GetTabLeft(inStartTab);
	
	if (inEndTab > mTabCount)
		r.right = GetTabRight(mTabCount);
	else
		r.right = GetTabRight(inEndTab);
	
	Refresh(r);
}

void CTabbedView::RefreshTab(Uint16 inTab)
{
	SRect r;
	GetTabRect(inTab, r);
	Refresh(r);
}

Uint16 CTabbedView::GetTabWidth(const void *inText, Uint32 inSize) const
{
	TImage img = UGraphics::GetDummyImage();
	img->SetFont(mFont);
	return UGraphics::GetTextWidth(img, inText, inSize) + 32;
}

Int32 CTabbedView::GetTabLeft(Uint16 inTab) const
{
	Require(IsValidTab(inTab));
	
	STabInfo *tabInfo;
	StHandleLocker lock(mTabInfo, (void*&)tabInfo);

	Int32 left = mBounds.left;
	Int16 i;
	
	inTab--;
	for (i=0; i<inTab; i++)
		left += tabInfo[i].width - 1;
	
	return left;
}

Int32 CTabbedView::GetTabRight(Uint16 inTab) const
{
	STabInfo *tabInfo;
	StHandleLocker lock(mTabInfo, (void*&)tabInfo);

	return GetTabLeft(inTab) + tabInfo[inTab-1].width;
}

void CTabbedView::GetTabRect(Uint16 inTab, SRect& outRect) const
{
	STabInfo *tabInfo;
	StHandleLocker lock(mTabInfo, (void*&)tabInfo);

	outRect.top = mBounds.top;
	outRect.bottom = outRect.top + mTabHeight;
	outRect.left = GetTabLeft(inTab);
	outRect.right = outRect.left + tabInfo[inTab-1].width;
}

Uint16 CTabbedView::PointToTab(const SPoint& inPt, SRect *outTabRect) const
{
	if (mTabCount && mTabInfo)
	{
		STabInfo *tabInfo;
		StHandleLocker lock(mTabInfo, (void*&)tabInfo);
		SRect r;
		Uint16 i;
		
		r.top = mBounds.top;
		r.bottom = r.top + mTabHeight;
		r.left = mBounds.left;
		
		for (i=0; i<mTabCount; i++)
		{
			r.right = r.left + tabInfo[i].width;
		
			if (r.Contains(inPt))
			{
				if (outTabRect) *outTabRect = r;
				return i+1;
			}
			
			r.left += tabInfo[i].width - 1;
		}
	}
	return 0;
}

Uint16 CTabbedView::GetTotalTabWidth() const
{
	if (mTabInfo)
	{
		STabInfo *tabInfo;
		StHandleLocker lock(mTabInfo, (void*&)tabInfo);

		Uint16 w = 0;
		
		for (Uint32 i=0; i<mTabCount; i++)
			w += tabInfo[i].width - 1;
		
		return w;
	}
	return 0;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CTabbedView::Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth)
{
	SRect r, tr;
	bool active = !mIsDrawDisabled;
	
	inImage->SetPenSize(1);
	
	r.top = mBounds.top + mTabHeight + 3;
	r.left = mBounds.left + 3;
	r.bottom = mBounds.bottom - 3;
	r.right = mBounds.right - 3;
	inImage->SetInkColor(color_GrayD);
	inImage->FillRect(r);
	
	r.top = mBounds.top + mTabHeight;
	r.left = mBounds.left;
	r.bottom = mBounds.bottom;
	r.right = mBounds.right;
	
	inImage->SetInkColor(active ? color_Black : color_Gray7);
	inImage->DrawLine(SLine(r.left, r.top, r.left, r.bottom-1), 1);
	inImage->DrawLine(SLine(r.right-1, r.top, r.right-1, r.bottom-1), 1);
	inImage->DrawLine(SLine(r.left, r.bottom-1, r.right-1, r.bottom-1), 1);
	
	inImage->SetInkColor(color_White);
	inImage->DrawLine(SLine(r.left+1, r.top+1, r.left+1, r.bottom-2), 1);
	inImage->DrawLine(SLine(r.left+2, r.top+1, r.left+2, r.bottom-3), 1);
	
	inImage->SetInkColor(active ? color_GrayA : color_GrayB);
	inImage->DrawLine(SLine(r.right-2, r.top+1, r.right-2, r.bottom-2), 1);
	inImage->DrawLine(SLine(r.right-3, r.top+1, r.right-3, r.bottom-2), 1);
	inImage->DrawLine(SLine(r.left+1, r.bottom-2, r.right-2, r.bottom-2), 1);
	inImage->DrawLine(SLine(r.left+2, r.bottom-3, r.right-2, r.bottom-3), 1);
	
	if (mTabCount == 0 || mTabInfo == nil)
	{
		// drawNoCurrentTab
		inImage->SetInkColor(active ? color_Black : color_Gray7);
		inImage->DrawLine(SLine(r.left, r.top, r.right-1, r.top), 1);
		
		inImage->SetInkColor(color_White);
		inImage->DrawLine(SLine(r.left+1, r.top+1, r.right-3, r.top+1), 1);
		inImage->DrawLine(SLine(r.left+1, r.top+2, r.right-4, r.top+2), 1);
		return;
	}
	
	STabInfo *tabInfo;
	StHandleLocker lock(mTabInfo, (void*&)tabInfo);
	SRect tabRect, curTabRect;
	THdl hTitle;
	Uint16 i;
	
	tabRect.top = mBounds.top;
	tabRect.left = mBounds.left;
	tabRect.bottom = tabRect.top + mTabHeight;
	inImage->SetFont(mFont);
	
	for (i=0; i<mTabCount; i++)
	{
		tabRect.right = tabRect.left + tabInfo[i].width;
	
		if (i+1 == mCurrentTab)
			curTabRect = tabRect;
		else if (tabRect.Intersects(inUpdateRect))
		{
			hTitle = tabInfo[i].title;
			if (hTitle)
			{
				void *titlep;
				StHandleLocker textLock(hTitle, titlep);
				if (mIsDrawDisabled)
					DrawDisabledTab(inImage, tabRect, titlep, UMemory::GetSize(hTitle), false);
				else
					DrawEnabledTab(inImage, tabRect, titlep, UMemory::GetSize(hTitle), mHilitedTab == i+1);
			}
			else
			{
				if (mIsDrawDisabled)
					DrawDisabledTab(inImage, tabRect, nil, 0, false);
				else
					DrawEnabledTab(inImage, tabRect, nil, 0, mHilitedTab == i+1);
			}
		}
		
		tabRect.left += tabInfo[i].width - 1;
	}
	
	if (mCurrentTab)
	{
		if (curTabRect.Intersects(inUpdateRect))
		{
			hTitle = tabInfo[mCurrentTab-1].title;
			if (hTitle)
			{
				UGraphics::SetFontEffect(inImage, fontEffect_Bold);
				void *titlep;
				StHandleLocker textLock(hTitle, titlep);
				if (mIsDrawDisabled)
					DrawDisabledTab(inImage, curTabRect, titlep, UMemory::GetSize(hTitle), true);
				else
					DrawEnabledTab(inImage, curTabRect, titlep, UMemory::GetSize(hTitle), true);
			}
			else
			{
				if (mIsDrawDisabled)
					DrawDisabledTab(inImage, curTabRect, nil, 0, true);
				else
					DrawEnabledTab(inImage, curTabRect, nil, 0, true);
			}
		}
		
		inImage->SetInkColor(active ? color_Black : color_Gray7);
		inImage->DrawLine(SLine(r.left, r.top, curTabRect.left, r.top), 1);
		inImage->DrawLine(SLine(curTabRect.right-1, r.top, r.right-1, r.top), 1);
		
		inImage->SetInkColor(color_White);
		inImage->DrawLine(SLine(r.left+1, r.top+1, curTabRect.left+2, r.top+1), 1);
		inImage->DrawLine(SLine(r.left+1, r.top+2, curTabRect.left+2, r.top+2), 1);
		inImage->DrawLine(SLine(curTabRect.right-3, r.top+1, r.right-3, r.top+1), 1);
		inImage->DrawLine(SLine(curTabRect.right-3, r.top+2, r.right-4, r.top+2), 1);
		
		inImage->SetInkColor(color_GrayD);
		tr.top = r.top;
		tr.bottom = tr.top + 3;
		tr.left = curTabRect.left+3;
		tr.right = curTabRect.right-3;
		inImage->FillRect(tr);
		
		inImage->SetPixel(SPoint(curTabRect.left+1, curTabRect.bottom), color_White);
		inImage->SetPixel(SPoint(curTabRect.left+2, curTabRect.bottom), color_White);
		inImage->SetPixel(SPoint(curTabRect.right-2, curTabRect.bottom), active ? color_GrayA : color_GrayB);
		inImage->SetPixel(SPoint(curTabRect.right-3, curTabRect.bottom), active ? color_GrayA : color_GrayB);
		
		// draw content
		CView *v = tabInfo[mCurrentTab-1].view;
		if (v)
		{
			SRect contentRect, localUpdateRect;
			
			// bring update rect into local coords
			GetContentRect(contentRect);
			localUpdateRect = inUpdateRect;
			localUpdateRect.Constrain(contentRect);
			localUpdateRect.top -= contentRect.top;
			localUpdateRect.left -= contentRect.left;
			localUpdateRect.bottom -= contentRect.top;
			localUpdateRect.right -= contentRect.left;

			v->GetBounds(tr);
			if (tr.Intersects(localUpdateRect))
			{
				// save and restore clipping and origin
				StClipSaver clipSave(inImage);
				StOriginSaver originSave(inImage);
				
				// clip to content and move clip so that it stays in same place after origin change
				UGraphics::IntersectClip(inImage, contentRect);
				UGraphics::MoveClip(inImage, -contentRect.left, -contentRect.top);
				
				// shift origin such that view draws according to left-top of content rect
				UGraphics::AddOrigin(inImage, contentRect.left, contentRect.top);
				
				// draw the view
				v->Draw(inImage, localUpdateRect, inDepth);
			}
		}
	}
	else
	{
drawNoCurrentTab:
		inImage->SetInkColor(active ? color_Black : color_Gray7);
		inImage->DrawLine(SLine(r.left, r.top, r.right-1, r.top), 1);
		
		inImage->SetInkColor(color_White);
		inImage->DrawLine(SLine(r.left+1, r.top+1, r.right-3, r.top+1), 1);
		inImage->DrawLine(SLine(r.left+1, r.top+2, r.right-4, r.top+2), 1);
	}
}

void CTabbedView::DrawTab(TImage inImage, const SRect& inRect, const void *inTitleData, Uint32 inTitleLen, const SColor& inFrameColor, const SColor& inContentColor, const SColor& inLightColor, const SColor& inDarkColor, const SColor& inTitleColor)
{
	SRect r;
		
	r = inRect;
	r.top += 3;
	r.left += 3;
	r.right -= 3;
	inImage->SetInkColor(inContentColor);
	inImage->FillRect(r);
	
	r = inRect;
	inImage->SetInkColor(inFrameColor);
	inImage->DrawLine(SLine(r.left, r.top+3, r.left, r.bottom-1), 1);
	inImage->DrawLine(SLine(r.right-1, r.top+3, r.right-1, r.bottom-1), 1);
	inImage->DrawLine(SLine(r.left+3, r.top, r.right-4, r.top), 1);
	inImage->DrawLine(SLine(r.left, r.top+3, r.left+3, r.top), 1);
	inImage->DrawLine(SLine(r.right-1, r.top+3, r.right-4, r.top), 1);
	
	inImage->SetInkColor(inLightColor);
	inImage->DrawLine(SLine(r.left+1, r.top+3, r.left+1, r.bottom-1), 1);
	inImage->DrawLine(SLine(r.left+2, r.top+2, r.left+2, r.bottom-1), 1);
	inImage->DrawLine(SLine(r.left+3, r.top+1, r.right-6, r.top+1), 1);
	inImage->DrawLine(SLine(r.left+3, r.top+2, r.right-6, r.top+2), 1);
	inImage->SetPixel(SPoint(r.left+3, r.top+3), inLightColor);
	
	inImage->SetInkColor(inDarkColor);
	inImage->DrawLine(SLine(r.right-2, r.top+3, r.right-2, r.bottom-1), 1);
	inImage->DrawLine(SLine(r.right-3, r.top+2, r.right-3, r.bottom-1), 1);
	inImage->SetPixel(SPoint(r.right-4, r.top+1), inDarkColor);
	inImage->SetPixel(SPoint(r.right-5, r.top+1), inDarkColor);
	inImage->SetPixel(SPoint(r.right-4, r.top+2), inDarkColor);
	inImage->SetPixel(SPoint(r.right-5, r.top+2), inDarkColor);
	inImage->SetPixel(SPoint(r.right-4, r.top+3), inDarkColor);

	if (inTitleData)
	{
		inImage->SetInkColor(inTitleColor);
#if MACINTOSH
		inImage->SetInkMode(mode_Or);
#endif
		UGraphics::DrawText(inImage, inRect, inTitleData, inTitleLen, 0, align_Center);
	}
}

void CTabbedView::DrawEnabledTab(TImage inImage, const SRect& inRect, const void *inTitleData, Uint32 inTitleLen, bool inCurrent)
{
	if (inCurrent)
		DrawTab(inImage, inRect, inTitleData, inTitleLen, color_Black, color_GrayD, color_White, color_GrayA, color_Black);
	else
		DrawTab(inImage, inRect, inTitleData, inTitleLen, color_Gray3, color_GrayC, color_GrayE, color_Gray9, color_Black);
}

void CTabbedView::DrawDisabledTab(TImage inImage, const SRect& inRect, const void *inTitleData, Uint32 inTitleLen, bool inCurrent)
{
	if (inCurrent)
		DrawTab(inImage, inRect, inTitleData, inTitleLen, color_Gray7, color_GrayD, color_White, color_GrayB, color_Gray7);
	else
		DrawTab(inImage, inRect, inTitleData, inTitleLen, color_Gray7, color_GrayD, color_GrayE, color_GrayB, color_Gray7);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CTabbedView::MouseDown(const SMouseMsgData& inInfo)
{
	CView::MouseDown(inInfo);
	
	Uint16 tab = 0;
	SRect r;
	
	mMouseDownInTabs = false;
	
	if (!mMouseCaptureTab)
		tab = PointToTab(inInfo.loc, &r);			// find which tab the mouse is in
#if DEBUG
	if (mMouseCaptureTab && mCurrentTab != mMouseCaptureTab)
		DebugBreak("MouseCapture tab != currentTab!");
#endif
		
	if (tab)										// if the mouse is in a tab
	{
		if (tab != mCurrentTab)						// don't hilite the current tab
		{
			mHilitedTab = tab;						// hilite the tab
			mMouseDownInTabs = true;
			Refresh(r);								// draw the tab
		}
	}
	else if (mCurrentTab)							// if we have a current tab
	{
		GetContentRect(r);
		if (r.Contains(inInfo.loc))					// if mouse is in content of tab
		{
			CView *v = GetTabView(mCurrentTab);
			if (v)									// if current tab has a view
			{
				SMouseMsgData info = inInfo;
				info.loc.v -= r.top;
				info.loc.h -= r.left;

				v->GetBounds(r);
				if (r.Contains(info.loc) || mMouseCaptureTab)			// if mouse is within views bounds
				{
					if (!v->IsMouseWithin())		// if mouse has not entered view
						v->MouseEnter(info);		// tell view mouse has entered
					v->MouseDown(info);				// let view handle the mouse down
				}
			}
		}
	}
}

void CTabbedView::MouseUp(const SMouseMsgData& inInfo)
{
	CView::MouseUp(inInfo);
	
	CView *v;
	SRect r;
	SMouseMsgData info;
	
	if (mHilitedTab)								// if we have a hilited tab
	{
		Uint16 tab = mHilitedTab;
		mHilitedTab = 0;							// unhilite tab
		
		if (PointToTab(inInfo.loc) == tab)			// if mouse went up in tab
		{
			if (mCurrentTab != tab)
			{
				if (mCurrentTab)
				{
					v = GetTabView(mCurrentTab);
					if (v)
					{
						GetContentRect(r);
						info = inInfo;
						info.loc.v -= r.top;
						info.loc.h -= r.left;
						
						if (v->IsAnyMouseBtnDown())
							v->MouseUp(info);
						if (v->IsMouseWithin())
							v->MouseLeave(info);
					}
				}
				SetCurrentTab(tab);					// make that tab current
			}
		}
		else
			RefreshTab(tab);						// redraw tab
	}
	else if (mCurrentTab)							// if we have a current tab
	{
		v = GetTabView(mCurrentTab);
		if (v)										// if current tab has a view
		{
			if (v->IsAnyMouseBtnDown())				// if we gave the mouse down to the view
			{
				GetContentRect(r);
				info = inInfo;
				info.loc.v -= r.top;
				info.loc.h -= r.left;
				v->MouseUp(info);					// let it have the mouse up
			}
		}
	}
}

void CTabbedView::MouseEnter(const SMouseMsgData& inInfo)
{
	CView::MouseEnter(inInfo);
	CTabbedView::MouseMove(inInfo);
}

void CTabbedView::MouseLeave(const SMouseMsgData& inInfo)
{
	CView::MouseLeave(inInfo);
	
	if (mHilitedTab)
	{
		Uint16 tab = mHilitedTab;
		mHilitedTab = 0;
		RefreshTab(tab);
	}
	
	if (mCurrentTab)
	{
		CView *v = GetTabView(mCurrentTab);
		if (v && v->IsMouseWithin())
		{
			SRect r;
			GetContentRect(r);
			SMouseMsgData info = inInfo;
			info.loc.v -= r.top;
			info.loc.h -= r.left;
			v->MouseLeave(info);
		}
	}
}

void CTabbedView::MouseMove(const SMouseMsgData& inInfo)
{
	CView::MouseMove(inInfo);
	
	if (IsAnyMouseBtnDown() && mMouseDownInTabs)
	{
		Uint16 tab = PointToTab(inInfo.loc);
		
		if (tab != mHilitedTab)
		{
			if (mHilitedTab)
			{
				Uint16 n = mHilitedTab;
				mHilitedTab = 0;
				RefreshTab(n);
			}
			
			if (tab && tab != mCurrentTab)
			{
				mHilitedTab = tab;
				RefreshTab(tab);
			}
		}
	}

#if DEBUG
	if (mMouseCaptureTab && mCurrentTab != mMouseCaptureTab)
		DebugBreak("MouseCapture tab != currentTab!");
#endif

	if (mCurrentTab)
	{
		CView *v = GetTabView(mCurrentTab);
		if (v)
		{
			SRect contentRect, viewRect;
			GetContentRect(contentRect);
			v->GetBounds(viewRect);
			
			SMouseMsgData info = inInfo;
			info.loc.v -= contentRect.top;
			info.loc.h -= contentRect.left;
			
			if (contentRect.Contains(inInfo.loc) && viewRect.Contains(info.loc))
			{
				if (v->IsMouseWithin())
					v->MouseMove(info);
				else
					v->MouseEnter(info);
			}
			else
			{
				if (v->IsMouseWithin())
					v->MouseLeave(info);
			}
		}
	}
}

bool CTabbedView::KeyDown(const SKeyMsgData& inInfo)
{
	if (mCurrentTab)
	{
		CView *v = GetTabView(mCurrentTab);
		if (v) return v->KeyDown(inInfo);
	}
	
	return false;
}

void CTabbedView::KeyUp(const SKeyMsgData& inInfo)
{
	if (mCurrentTab)
	{
		CView *v = GetTabView(mCurrentTab);
		if (v) v->KeyUp(inInfo);
	}
}

bool CTabbedView::KeyRepeat(const SKeyMsgData& inInfo)
{
	if (mCurrentTab)
	{
		CView *v = GetTabView(mCurrentTab);
		if (v) return v->KeyRepeat(inInfo);
	}
	
	return false;
}

void CTabbedView::UpdateActive()
{
	bool drawDisab = IsDisabled() || IsInactive();
	
	if (drawDisab != mIsDrawDisabled)	// if change
	{
		mIsDrawDisabled = drawDisab;
		Refresh();
	}
}

bool CTabbedView::ChangeState(Uint16 inState)
{
	if (mState != inState)	// if change
	{
		mState = inState;
		UpdateActive();
		
		// only need to change the state of the current tabs view (the other views stay viewState_Hidden)
		if (mCurrentTab)
		{
			CView *v = GetTabView(mCurrentTab);
			if (v) v->ChangeState(inState);
		}
		
		return true;
	}
	
	return false;
}


bool CTabbedView::TabFocusPrev()
{
	if(!(mCanFocus && mIsVisible && mIsEnabled))
		return false;

	CView *v = GetTabView(mCurrentTab);

	if(v ? v->TabFocusPrev() : false)
	{
		if(mState == viewState_Active)
			ChangeState(viewState_Focus);
		
		return true;
	}
	else
	{
		if(mState == viewState_Focus)
			ChangeState(viewState_Active);
		
		return false;
	}
}

bool CTabbedView::TabFocusNext()
{
	if(!(mCanFocus && mIsVisible && mIsEnabled))
		return false;

	CView *v = GetTabView(mCurrentTab);

	if(v ? v->TabFocusNext() : false)
	{
		if(mState == viewState_Active)
			ChangeState(viewState_Focus);
		
		return true;
	}
	else
	{
		if(mState == viewState_Focus)
			ChangeState(viewState_Active);
		
		return false;
	}
}

bool CTabbedView::SetEnable(bool inEnable)
{
	if (mIsEnabled != (inEnable != 0))		// if change
	{
		mIsEnabled = (inEnable != 0);
		UpdateActive();
		
		return true;
	}
	
	return false;
}

bool CTabbedView::SetBounds(const SRect& inBounds)
{
	Int32 oldHeight = mBounds.GetHeight();
	Int32 oldWidth = mBounds.GetWidth();

	if (CView::SetBounds(inBounds))
	{
		SRect r;
		CView *v;
		Uint32 i = 0;
		Uint16 sizing;
		
		Int32 newHeight = inBounds.GetHeight();
		Int32 newWidth = inBounds.GetWidth();
				
		if (oldHeight != newHeight || oldWidth != newWidth)
		{
			while (mViewList.GetNext(v, i))
			{
				sizing = v->GetSizing();
				if (sizing)
				{
					v->GetBounds(r);
					
					if (sizing & sizing_LeftSticky)
						r.left = newWidth - (oldWidth - r.left);
					
					if (sizing & sizing_RightSticky)
						r.right = newWidth - (oldWidth - r.right);
					
					if (sizing & sizing_TopSticky)
						r.top = newHeight - (oldHeight - r.top);
						
					if (sizing & sizing_BottomSticky)
						r.bottom = newHeight - (oldHeight - r.bottom);

					v->SetBounds(r);
				}
								
				// if is a QuickTimeView we need to update the movie bounds
				CQuickTimeView *pQuickTimeView = dynamic_cast<CQuickTimeView *>(v);
				if (pQuickTimeView)
					pQuickTimeView->UpdateQuickTimeBounds();
			}
		}
	
		return true;
	}
	
	return false;
}

#if 0
bool CTabbedView::SetBounds(const SRect& inBounds)
{
	Int32 oldHeight = mBounds.GetHeight();
	Int32 oldWidth = mBounds.GetWidth();

	if (CView::SetBounds(inBounds))
	{
		SRect r;
		CView *v;
		Uint32 i = 0;
		Uint16 sizing;
		
		Int32 newHeight = inBounds.GetHeight();
		Int32 newWidth = inBounds.GetWidth();
				
		if (oldHeight != newHeight || oldWidth != newWidth)
		{
			while (mViewList.GetNext(v, i))
			{
				sizing = v->GetSizing();
				if (sizing)
				{
					v->GetBounds(r);
					
					if (sizing & sizing_LeftSticky)
						r.left = newWidth - (oldWidth - r.left);
					
					if (sizing & sizing_RightSticky)
						r.right = newWidth - (oldWidth - r.right);
					
					if (sizing & sizing_TopSticky)
						r.top = newHeight - (oldHeight - r.top);
						
					if (sizing & sizing_BottomSticky)
						r.bottom = newHeight - (oldHeight - r.bottom);

					v->SetBounds(r);
				}
			}
		}
		
		return true;
	}
	
	return false;
}
#endif

void CTabbedView::MouseCaptureReleased()
{
	CView::MouseCaptureReleased();
	
	mMouseCaptureTab = 0;
	if (mMouseCaptureView)
	{
		mMouseCaptureView->MouseCaptureReleased();
		mMouseCaptureView = nil;
	}
}

void CTabbedView::DragEnter(const SDragMsgData& inInfo)
{
	CView::DragEnter(inInfo);
	CTabbedView::DragMove(inInfo);		// don't use the virtual dispatch
}

void CTabbedView::DragMove(const SDragMsgData& inInfo)
{
	CView::DragMove(inInfo);
	
	if (mCurrentTab)
	{
		CView *v = GetTabView(mCurrentTab);
		if (v)
		{
			SRect contentRect, viewRect;
			GetContentRect(contentRect);
			v->GetBounds(viewRect);
			
			SDragMsgData info = inInfo;
			info.loc.v -= contentRect.top;
			info.loc.h -= contentRect.left;
			
			if (contentRect.Contains(inInfo.loc) && viewRect.Contains(info.loc))
			{
				if (v->IsDragWithin())
				{
					//if (mDragWithinViewCanAccept)
						v->DragMove(info);
				}
				else
				{
//					mDragWithinViewCanAccept = (v->CanAcceptDrop(inInfo.drag) != 0);
//					if (mDragWithinViewCanAccept)
						v->DragEnter(info);
				}
			}
			else
			{
				if (v->IsDragWithin())
				{
					//if (mDragWithinViewCanAccept)
						v->DragLeave(info);
				}
			}
		}
	}
}

void CTabbedView::DragLeave(const SDragMsgData& inInfo)
{
	CView::DragLeave(inInfo);
	
	if (mCurrentTab)// && mDragWithinViewCanAccept)
	{
		CView *v = GetTabView(mCurrentTab);
		if (v && v->IsDragWithin())
		{
			SRect r;
			GetContentRect(r);
			SDragMsgData info = inInfo;
			info.loc.v -= r.top;
			info.loc.h -= r.left;
			v->DragLeave(info);
		}
	}
}

bool CTabbedView::Drop(const SDragMsgData& inInfo)
{
	if (mCurrentTab)// && mDragWithinViewCanAccept)
	{
		CView *v = GetTabView(mCurrentTab);
		if (v && v->IsDragWithin())
		{
			SRect r;
			GetContentRect(r);
			SDragMsgData info = inInfo;
			info.loc.v -= r.top;
			info.loc.h -= r.left;
			return v->Drop(info);
		}
	}
	return false;
}

/*
bool CTabbedView::CanAcceptDrop(TDrag inDrag) const
{
	if (mCurrentTab)
	{
		CView *v = GetTabView(mCurrentTab);
		if (v) return v->CanAcceptDrop(inDrag);
	}
	return false;
}
*/

/* -------------------------------------------------------------------------- */
#pragma mark -

void CTabbedView::UpdateQuickTime()
{
	Uint32 i = 0;
	CView *pView;
		
	while (mViewList.GetNext(pView, i))
	{
		if (pView->IsVisible())								// ignore invisible views
			pView->UpdateQuickTime();
	}
}

void CTabbedView::SendToQuickTime(const EventRecord& inInfo)
{
	Uint32 i = 0;
	CView *pView;
		
	while (mViewList.GetNext(pView, i))
	{
		if (pView->IsVisible())								// ignore invisible views
			pView->SendToQuickTime(inInfo);
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CTabbedView::HandleRemove(CView *inView)
{
	CMultiViewContainer::HandleRemove(inView);
	
	if (mTabInfo)
	{
		STabInfo *tabInfo;
		StHandleLocker lock(mTabInfo, (void*&)tabInfo);

		Uint16 i;
		
		// if ANY of the installed views match <inView>, detach them
		for (i=0; i<mTabCount; i++)
		{
			if (tabInfo[i].view == inView)
			{
				tabInfo[i].view = nil;
			
				// remove from data members
				if (mFocusView == inView)			mFocusView = nil;
				if (mMouseCaptureView == inView)	
				{
					mMouseCaptureTab = 0;
					mMouseCaptureView = nil;
				}
			}
		}
	}
}

void CTabbedView::HandleRefresh(CView *inView, const SRect& inUpdateRect)
{
	if (mCurrentTab && inView == GetTabView(mCurrentTab))
	{
		SRect updateRect, contentRect;
		
		GetContentRect(contentRect);
		
		updateRect = inUpdateRect;
		updateRect.Move(contentRect.left, contentRect.top);
		
		Refresh(updateRect);
	}
}

void CTabbedView::HandleHit(CView *inView, const SHitMsgData& inInfo)
{
	if (mHandler) mHandler->HandleHit(inView, inInfo);
}

void CTabbedView::HandleGetScreenDelta(CView */* inView */, Int32& outHoriz, Int32& outVert)
{
	SRect stContentRect;
	GetContentRect(stContentRect);

	GetScreenDelta(outHoriz, outVert);
	
	outHoriz += stContentRect.left;
	outVert += stContentRect.top;
}

void CTabbedView::HandleGetVisibleRect(CView */* inView */, SRect& outRect)
{	
	SRect stVisibleRect;
	GetVisibleRect(stVisibleRect);

	SRect stContentRect;
	GetContentRect(stContentRect);
	
	stVisibleRect.GetIntersection(stContentRect, outRect);
	outRect.Move(-mBounds.left, -mBounds.top);
}

void CTabbedView::HandleCaptureMouse(CView *inView)
{
	if (mTabInfo)
	{
		STabInfo *tabInfo;
		StHandleLocker lock(mTabInfo, (void*&)tabInfo);

		Uint16 i;
		
		// if ANY of the installed views match <inView>, detach them
		for (i=0; i<mTabCount; i++)
		{
			if (tabInfo[i].view == inView)
			{
				mMouseCaptureTab = i + 1;
				mMouseCaptureView = inView;
				if (mHandler) mHandler->HandleCaptureMouse(this);
				break;
			}
		}
	}
}

void CTabbedView::HandleReleaseMouse(CView *inView)
{
	if (mMouseCaptureView == inView)
	{	if (mHandler) mHandler->HandleReleaseMouse(this);	}	
}

void CTabbedView::HandleGetOrigin(SPoint& outOrigin)
{
	if (mHandler) mHandler->HandleGetOrigin(outOrigin);

	outOrigin.x += mBounds.left;
	outOrigin.y += mBounds.top;
}

CView *CTabbedView::HandleGetCaptureMouseView()
{
	return mMouseCaptureView;
}

CWindow *CTabbedView::HandleGetParentWindow()
{
	return GetParentWindow();
}

