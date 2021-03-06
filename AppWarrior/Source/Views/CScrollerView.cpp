/***************************************************************************

Scrolling title bars / headers

Need a way of having a horiz/vert title bar at the top/left of the CScrollerView.
For example, like the titles in the Finder when a folder is in By Name view.
Or like the titles in a spreadsheet.  Furthermore, these titles need to scroll
as the content scrolls.

Solution:  add data members mHorizTitleBarSize and mVertTitleBarSize.  These
are 0 by default.  IntCalcRects() adds space for the bars, and calcs rects
for them.

If the sizes are not 0, Draw() calls IntDrawHoriz/VertTitleBar().
Remember that these bars must scroll along with the content.
Will also need a IntTitleBarMouseDown() etc.

Or perhaps,  IntTitleMouseDown(bool inIsHoriz)

**** should be use views for the titles??  (header is a better word)
And then we just forward the mousedown/draw/whatever to the view.
CScrollerView would then have to be CMultiViewContainer instead of a
CSingleViewContainer.  Maybe have CScrollerView and CHeaderScrollerView ?
But what if you want to draw something in those corners?  eg a button
in the topRight that toggles the sorting order (ascending/descending).

********* I've got it! *************

Have CAbstractScrollerView.  For drawing the content, this has a pure
virtual function which derived classes must override to draw the content.
Also virtual functions for drawing the headers, corners, handling mouse
events etc.

CScrollerView scrolls another view.  So to implement, derive CScrollerView
from CAbstractScrollerView, and override the virtual "draw-content"
function and call the other view to do the drawing.

For a scrolling text box, you could either put a CTextView inside a
CScrollerView as is done currently, OR make a CTextBoxView that is
derived from CAbstractScrollerView, and uses UEditText to draw the
content etc.

For an all-in-one class that implements a scrolling list with a header
and corners, you could derive a class from CAbstractScrollerView, and
use UFixedSizeList (or whatever) to draw the content, and UColumnHeader
(or whatever) to draw the header.  Man, that kicks ass!

******************************************

CAbstractScrollerView
	- calls virtual funcs to draw contents etc

CScrollerView : CAbstractScrollerView
	- scrolls any view

CTextBoxView : CAbstractScrollerView
	- uses UEditText to draw contents

CTextView : CView
	- uses UEditTest to draw but does NOT scroll

****************************************************************************/





/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "CScrollerView.h"
#include "UScrollBar.h"

#pragma options align=packed

typedef struct {
	Uint32 hScrollVal, vScrollVal;
	Uint16 options;
	Uint16 margin;
	SColor color;
	Uint32 rsvd[4];
} SScrollerView;

#pragma options align=reset

/* -------------------------------------------------------------------------- */

CScrollerView::CScrollerView(CViewHandler *inHandler, const SRect& inBounds)
	: CView(inHandler, inBounds), mDestRect(0,0,0,0), mContentRect(0,0,0,0)
{
	mOptions = scrollerOption_VertBar | scrollerOption_Border;
	mRectsValid = mValuesValid = false;
	mHScrollEnabled = mVScrollEnabled = false;
	mIsDrawDisabled = mIsDrawFocus = false;
	mMouseCapture = false;
	mHScrollPart = mVScrollPart = 0;
	mMargin = 0;
	mScrollTimer = nil;
	mContentColor.red = mContentColor.green = mContentColor.blue = 0xFFFF;
}

CScrollerView::CScrollerView(CViewHandler *inHandler, const SRect& inBounds, Uint32 inOptions, Uint16 inMargin)
	: CView(inHandler, inBounds), mDestRect(0,0,0,0), mContentRect(0,0,0,0)
{
	mOptions = inOptions;
	mRectsValid = mValuesValid = false;
	mHScrollEnabled = mVScrollEnabled = false;
	mIsDrawDisabled = mIsDrawFocus = false;
	mMouseCapture = false;
	mHScrollPart = mVScrollPart = 0;
	mMargin = inMargin;
	mScrollTimer = nil;
	mContentColor.red = mContentColor.green = mContentColor.blue = 0xFFFF;
}

CScrollerView::~CScrollerView()
{
	delete mScrollTimer;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CScrollerView::SetOptions(Uint16 inOptions)
{
	if (mOptions != inOptions)
	{
		mOptions = inOptions;
		mRectsValid = mValuesValid = false;
		Refresh();
	}
}

Uint16 CScrollerView::GetOptions() const
{
	return mOptions;
}

void CScrollerView::SetCanFocus(bool inCanFocus)
{
	if (mCanFocus != (inCanFocus!=0))		// if change
	{
		CView::SetCanFocus(inCanFocus);
		mRectsValid = false;
		Refresh();
	}
}

void CScrollerView::SetMargin(Uint16 inSpace)
{
	if (mMargin != inSpace)
	{
		mMargin = inSpace;
		mRectsValid = false;
		Refresh();
	}
}

Uint16 CScrollerView::GetMargin() const
{
	return mMargin;
}

Int32 CScrollerView::GetVisibleContentHeight() const
{
	((CScrollerView *)this)->CalcRects();
	return mContentRect.GetHeight();
}

Int32 CScrollerView::GetVisibleContentWidth() const
{
	((CScrollerView *)this)->CalcRects();
	return mContentRect.GetWidth();
}

void CScrollerView::SetContentColor(const SColor& inColor)
{
	if (inColor != mContentColor)
	{
		mContentColor = inColor;
		Refresh();
	}
}

void CScrollerView::GetContentColor(SColor& outColor)
{
	outColor = mContentColor;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CScrollerView::SetScroll(Uint32 inHorizPos, Uint32 inVertPos)
{
	CalcValues();
	
	if (!mHScrollEnabled)
		inHorizPos = 0;
	else if (inHorizPos > mHScrollMax)
		inHorizPos = mHScrollMax;
	
	if (!mVScrollEnabled)
		inVertPos = 0;
	else if (inVertPos > mVScrollMax)
		inVertPos = mVScrollMax;
	
	SRect newDest;
	
	newDest.top = mContentRect.top - inVertPos;
	newDest.left = mContentRect.left - inHorizPos;
	newDest.bottom = newDest.top + mDestRect.GetHeight();
	newDest.right = newDest.left + mDestRect.GetWidth();
	
	if ((newDest.top > mContentRect.top) || (newDest.left > mContentRect.left))
		newDest.MoveTo(mContentRect.left, mContentRect.top);
		
	if (newDest != mDestRect)
	{
		mDestRect = newDest;
		mValuesValid = false;

		if (mView)
			Refresh(mContentRect);
		
		CalcValuesAndRefresh();
	}
}

void CScrollerView::GetScroll(Uint32& outHorizPos, Uint32& outVertPos) const
{
	((CScrollerView *)this)->CalcValues();
	outHorizPos = mHScrollVal;
	outVertPos = mVScrollVal;
}

// positive values scroll right or down, and negative values scroll left or up
void CScrollerView::Scroll(Int32 inHorizDelta, Int32 inVertDelta)
{
	Uint32 h, v;
	GetScroll(h, v);
	
	inHorizDelta += h;
	inVertDelta += v;
	
	if (inHorizDelta < 0) inHorizDelta = 0;
	if (inVertDelta < 0) inVertDelta = 0;
	
	SetScroll(inHorizDelta, inVertDelta);
}

void CScrollerView::ScrollToTop()
{
	Uint32 h, v;
	GetScroll(h, v);
	SetScroll(h, 0);
}

void CScrollerView::ScrollToLeft()
{
	Uint32 h, v;
	GetScroll(h, v);
	SetScroll(0, v);
}

void CScrollerView::ScrollToBottom()
{
	Uint32 h, v;
	GetScroll(h, v);
	SetScroll(h, max_Uint32);
}

void CScrollerView::ScrollToRight()
{
	Uint32 h, v;
	GetScroll(h, v);
	SetScroll(max_Uint32, v);
}

// inRect is in coords local to the content view
void CScrollerView::ScrollToRect(const SRect& inRect, Uint16 inAlign)
{
	SRect r, cr;
	
	// make sure the default is on (inside is checked last)
	inAlign |= align_InsideHoriz | align_InsideVert;
	
	// convert content rect to same coords as <inRect>
	cr = mContentRect;
	cr.top -= mDestRect.top;
	cr.left -= mDestRect.left;
	cr.bottom -= mDestRect.top;
	cr.right -= mDestRect.left;
	
	// get outta here if already in view
	if ((inAlign & align_NotIfInside) && cr.Contains(inRect))
		return;
	
	// align <inRect> to content
	r = inRect;
	r.Align(inAlign, cr);
	
	// make r local to cr
	r.top -= cr.top;
	r.left -= cr.left;
	
	// scroll to place the content around <inRect>
	SetScroll(inRect.left - r.left, inRect.top - r.top);
}

bool CScrollerView::IsScrolledToBottom()
{
	return mVScrollVal >= mVScrollMax;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void _SetVirtualOrigin(TImage inImage, const SPoint& inVirtualOrigin);
void _GetVirtualOrigin(TImage inImage, SPoint& outVirtualOrigin);
void _AddVirtualOrigin(TImage inImage, const SPoint& inVirtualOrigin);

void CScrollerView::Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth)
{
	SRect r;
	SScrollBarInfo info;

	CalcRects();
	CalcValues();
	
	// save and restore background color
#if MACINTOSH
	StBackColorSaver saveBackColor(inImage);
#endif

	// white wash content if applicable
	if ( ((mOptions & scrollerOption_NoBkgnd) == 0) && ((mOptions & scrollerOption_Border) || (mOptions & scrollerOption_PlainBorder)) )
	{
#if MACINTOSH
		UGraphics::SetBackColor(inImage, mContentColor);
#endif
		r = mContentRect;
		r.Enlarge(mMargin, mMargin);
		r.GetIntersection(inUpdateRect, r);
		inImage->SetInkColor(mContentColor);
		inImage->FillRect(r);
	}
	
	// draw view
	if (mView)
	{					
		SRect stLocalUpdateRect = inUpdateRect;	
		stLocalUpdateRect.GetIntersection(mContentRect, stLocalUpdateRect);
		
		if (stLocalUpdateRect.IsNotEmpty())
		{		
			// save and restore clipping and origin
			StClipSaver stClipSave(inImage);
			StOriginSaver stOriginSave(inImage);

			// save virtual origin
			SPoint stVirtualOriginSave;
			_GetVirtualOrigin(inImage, stVirtualOriginSave);

			// calculate virtual origin
			SPoint stOrigin;
			UGraphics::GetOrigin(inImage, stOrigin);
			
			Int32 nOriginLimit = 7*max_Int16/16;	// on Win95 the limit must be less that max_Int16/2

			SPoint stVirtualOrigin;
			ClearStruct(stVirtualOrigin);
						
			if (stOrigin.x - mDestRect.left > nOriginLimit)
				stVirtualOrigin.x = stOrigin.x - mDestRect.left - nOriginLimit;

			if (stOrigin.y - mDestRect.top > nOriginLimit)
				stVirtualOrigin.y = stOrigin.y - mDestRect.top - nOriginLimit;
			
			// add virtual origin
			_AddVirtualOrigin(inImage, stVirtualOrigin);

			// clip to content and move clip so that it stays in same place after origin change
			UGraphics::IntersectClip(inImage, stLocalUpdateRect);
			UGraphics::MoveClip(inImage, -(mDestRect.left + stVirtualOrigin.x), -(mDestRect.top + stVirtualOrigin.y));
		
			// shift origin such that view draws according to left-top of destination rect
			UGraphics::AddOrigin(inImage, mDestRect.left + stVirtualOrigin.x, mDestRect.top + stVirtualOrigin.y);

			// bring update rect into local coords
			stLocalUpdateRect.top -= mDestRect.top;
			stLocalUpdateRect.left -= mDestRect.left;
			stLocalUpdateRect.bottom -= mDestRect.top;
			stLocalUpdateRect.right -= mDestRect.left;

			// draw the view
			mView->Draw(inImage, stLocalUpdateRect, inDepth);

			// restore virtual origin
			_SetVirtualOrigin(inImage, stVirtualOriginSave);
		}
	}
	
	// draw drag hilite
	if (mIsDragHilited)
	{
		r = mContentRect;
		r.Enlarge(mMargin, mMargin);
		UDragAndDrop::DrawHilite(inImage, r);
	}
	
	// draw everything else
	if (!mContentRect.Contains(inUpdateRect))
	{
		// draw border
		if (mOptions & scrollerOption_Border)
		{
			UUserInterface::DrawStandardBox(inImage, mBounds, nil, !mIsEnabled, mCanFocus && (!(mOptions & scrollerOption_NoFocusBorder)), mIsDrawFocus);
		}
		else if (mOptions & scrollerOption_PlainBorder)
		{
			r = mBounds;
			if (mCanFocus && (!(mOptions & scrollerOption_NoFocusBorder))) r.Enlarge(-1, -1);
			
			inImage->SetPenSize(1);
			inImage->SetInkColor(color_Black);
			inImage->FrameRect(r);
		}
				
		// draw vertical scroll bar
		if (mOptions & scrollerOption_VertBar && mVScrollRect.Intersects(inUpdateRect))
		{
			info.val = mVScrollVal;
			info.max = mVScrollMax;
			info.visVal = mContentRect.GetHeight();
			info.visMax = mDestRect.GetHeight();
			info.options = (inDepth >= 4) ? 0 : 0x8000;
			
			if (mVScrollEnabled && !mIsDrawDisabled)
				UScrollBar::Draw(inImage, mVScrollRect, info, mVScrollPart);
			else
				UScrollBar::DrawDisabled(inImage, mVScrollRect, info);
		}
		
		// draw horizontal scroll bar
		if (mOptions & scrollerOption_HorizBar && mHScrollRect.Intersects(inUpdateRect))
		{
			info.val = mHScrollVal;
			info.max = mHScrollMax;
			info.visVal = mContentRect.GetWidth();
			info.visMax = mDestRect.GetWidth();
			info.options = (inDepth >= 4) ? 0 : 0x8000;
			
			if (mHScrollEnabled && !mIsDrawDisabled)
				UScrollBar::Draw(inImage, mHScrollRect, info, mHScrollPart);
			else
				UScrollBar::DrawDisabled(inImage, mHScrollRect, info);
		}
	}
}

void CScrollerView::MouseDown(const SMouseMsgData& inInfo)
{
	CView::MouseDown(inInfo);
	
	SPoint loc = inInfo.loc;
	SRect r;
	Uint16 part;
	SScrollBarInfo scrollInfo;
	
	CalcValues();
	
	// get outta here if not active or enabled
	if (IsDisabled() || !IsActive())
		return;

	// find which area the mouse is in
	if (mContentRect.Contains(loc))
	{
		if (mView)
		{
			SMouseMsgData info = inInfo;
			
			// bring mouse location into local coords
			info.loc.v -= mDestRect.top;
			info.loc.h -= mDestRect.left;
						
			// let view handle mouse down
			mView->MouseDown(info);
		}
	}
	else if (mOptions & scrollerOption_VertBar && mVScrollRect.Contains(loc))
	{
		if (mVScrollEnabled)
		{
			scrollInfo.val = mVScrollVal;
			scrollInfo.max = mVScrollMax;
			scrollInfo.visVal = mContentRect.GetHeight();
			scrollInfo.visMax = mDestRect.GetHeight();
			scrollInfo.options = 0;

			part = UScrollBar::PointToPart(mVScrollRect, scrollInfo, loc);
			if (part != mVScrollPart)
			{
				mVScrollPart = part;
				
				switch (part)
				{
					case sbPart_Thumb:
						mThumbDelta = UScrollBar::GetThumbDelta(mVScrollRect, scrollInfo, loc);
						break;
					case sbPart_Up:
					case sbPart_SecondUp:
						Scroll(0, -16);
						break;
					case sbPart_Down:
					case sbPart_SecondDown:
						Scroll(0, 16);
						break;
				}

				UScrollBar::GetPartRect(mVScrollRect, scrollInfo, part, r);
				Refresh(r);
				
				switch (part)
				{
					case sbPart_Up:
					case sbPart_Down:
					case sbPart_SecondUp:
					case sbPart_SecondDown:
						if (mScrollTimer == nil)
							mScrollTimer = NewTimer();
						mScrollTimer->Start(25, kRepeatingTimer);
						break;
						
					case sbPart_PageUp:
					case sbPart_PageDown:
						mScrollTimerWentOff = false;
						if (mScrollTimer == nil)
							mScrollTimer = NewTimer();
						mScrollTimer->Start(100, kRepeatingTimer);
						break;
				}
			}
		}
	}
	else if (mOptions & scrollerOption_HorizBar && mHScrollRect.Contains(loc))
	{
		if (mHScrollEnabled)
		{
			scrollInfo.val = mHScrollVal;
			scrollInfo.max = mHScrollMax;
			scrollInfo.visVal = mContentRect.GetWidth();
			scrollInfo.visMax = mDestRect.GetWidth();;
			scrollInfo.options = 0;

			part = UScrollBar::PointToPart(mHScrollRect, scrollInfo, loc);
			if (part != mHScrollPart)
			{
				mHScrollPart = part;
				
				switch (part)
				{
					case sbPart_Thumb:
						mThumbDelta = UScrollBar::GetThumbDelta(mHScrollRect, scrollInfo, loc);
						break;
					case sbPart_Left:
					case sbPart_SecondLeft:
						Scroll(-16, 0);
						break;
					case sbPart_Right:
					case sbPart_SecondRight:
						Scroll(16, 0);
						break;
				}

				UScrollBar::GetPartRect(mHScrollRect, scrollInfo, part, r);
				Refresh(r);
				
				switch (part)
				{
					case sbPart_Left:
					case sbPart_Right:
					case sbPart_SecondLeft:
					case sbPart_SecondRight:
						if (mScrollTimer == nil)
							mScrollTimer = NewTimer();
						mScrollTimer->Start(25, kRepeatingTimer);
						break;
						
					case sbPart_PageLeft:
					case sbPart_PageRight:
						mScrollTimerWentOff = false;
						if (mScrollTimer == nil)
							mScrollTimer = NewTimer();
						mScrollTimer->Start(100, kRepeatingTimer);
						break;
				}
			}
		}
	}
	else if (mMouseCapture)
	{
		if (mView)
		{
			SMouseMsgData info = inInfo;
			
			// bring mouse location into local coords
			info.loc.v -= mDestRect.top;
			info.loc.h -= mDestRect.left;
						
			// let view handle mouse down
			mView->MouseDown(info);
		}
	}
}

void CScrollerView::MouseUp(const SMouseMsgData& inInfo)
{
	CView::MouseUp(inInfo);
	
	SRect r;
	SScrollBarInfo scrollInfo;
	
	CalcValues();
	
	if (mVScrollPart)
	{
		switch (mVScrollPart)
		{
			case sbPart_Up:
			case sbPart_Down:
			case sbPart_SecondUp:
			case sbPart_SecondDown:
				if (mScrollTimer) mScrollTimer->Stop();
				break;
				
			case sbPart_PageUp:
				if (mScrollTimer) mScrollTimer->Stop();
				if (!mScrollTimerWentOff) Scroll(0, -mContentRect.GetHeight());
				break;
				
			case sbPart_PageDown:
				if (mScrollTimer) mScrollTimer->Stop();
				if (!mScrollTimerWentOff) Scroll(0, mContentRect.GetHeight());
				break;
		}
		
		scrollInfo.val = mVScrollVal;
		scrollInfo.max = mVScrollMax;
		scrollInfo.visVal = mContentRect.GetHeight();
		scrollInfo.visMax = mDestRect.GetHeight();
		scrollInfo.options = 0;
		UScrollBar::GetPartRect(mVScrollRect, scrollInfo, mVScrollPart, r);
		Refresh(r);
		mVScrollPart = 0;
	}
	else if (mHScrollPart)
	{
		switch (mHScrollPart)
		{
			case sbPart_Left:
			case sbPart_Right:
			case sbPart_SecondLeft:
			case sbPart_SecondRight:
				if (mScrollTimer) mScrollTimer->Stop();
				break;
				
			case sbPart_PageLeft:
				if (mScrollTimer) mScrollTimer->Stop();
				if (!mScrollTimerWentOff) Scroll(-mContentRect.GetHeight(), 0);
				break;
				
			case sbPart_PageRight:
				if (mScrollTimer) mScrollTimer->Stop();
				if (!mScrollTimerWentOff) Scroll(mContentRect.GetHeight(), 0);
				break;
		}
		
		scrollInfo.val = mHScrollVal;
		scrollInfo.max = mHScrollMax;
		scrollInfo.visVal = mContentRect.GetWidth();
		scrollInfo.visMax = mDestRect.GetWidth();
		scrollInfo.options = 0;
		UScrollBar::GetPartRect(mHScrollRect, scrollInfo, mHScrollPart, r);
		Refresh(r);
		mHScrollPart = 0;
	}
	
	if (mView && mView->IsAnyMouseBtnDown())
	{
		SMouseMsgData info = inInfo;
		info.loc.v -= mDestRect.top;
		info.loc.h -= mDestRect.left;
		mView->MouseUp(info);
	}
}

void CScrollerView::MouseEnter(const SMouseMsgData& inInfo)
{
	CView::MouseEnter(inInfo);

	// in this case, mouse enter is the same as move
	CScrollerView::MouseMove(inInfo);					// don't use the virtual dispatch
}

void CScrollerView::MouseMove(const SMouseMsgData& inInfo)
{
	CView::MouseMove(inInfo);

	SScrollBarInfo scrollInfo;
	Uint32 val;
	
	// get outta here if not active or enabled
	if (IsDisabled() || !IsActive())
		return;

	CalcValues();
	
	if (mVScrollPart == sbPart_Thumb)
	{
		scrollInfo.val = mVScrollVal;
		scrollInfo.max = mVScrollMax;
		scrollInfo.visVal = mContentRect.GetHeight();
		scrollInfo.visMax = mDestRect.GetHeight();
		scrollInfo.options = 0;
		val = UScrollBar::GetThumbValue(mVScrollRect, scrollInfo, inInfo.loc, mThumbDelta);
		SetScroll(mHScrollVal, val);
	}
	else if (mHScrollPart == sbPart_Thumb)
	{
		scrollInfo.val = mHScrollVal;
		scrollInfo.max = mHScrollMax;
		scrollInfo.visVal = mContentRect.GetWidth();
		scrollInfo.visMax = mDestRect.GetWidth();
		scrollInfo.options = 0;
		val = UScrollBar::GetThumbValue(mHScrollRect, scrollInfo, inInfo.loc, mThumbDelta);
		SetScroll(val, mVScrollVal);
	}
	else
	{		
		if (mView)
		{
			SMouseMsgData info = inInfo;
			info.loc.v -= mDestRect.top;
			info.loc.h -= mDestRect.left;
		
			if (mContentRect.Contains(inInfo.loc) || mMouseCapture)
			{
				if (mView->IsMouseWithin())
					mView->MouseMove(info);
				else
					mView->MouseEnter(info);
			}
			else
			{
				if (mView->IsMouseWithin())
					mView->MouseLeave(info);
			}
		}
	}
}

void CScrollerView::MouseLeave(const SMouseMsgData& inInfo)
{
	CView::MouseLeave(inInfo);
	
	if (mView && mView->IsMouseWithin())
	{
		CalcRects();
		
		SMouseMsgData info = inInfo;
		info.loc.v -= mDestRect.top;
		info.loc.h -= mDestRect.left;
		mView->MouseLeave(info);
	}
}

bool CScrollerView::KeyDown(const SKeyMsgData& inInfo)
{
	switch (inInfo.keyCode)
	{
		case key_PageUp:
			Scroll(0, -mContentRect.GetHeight());
			break;
		case key_PageDown:
			Scroll(0, mContentRect.GetHeight());
			break;
		case key_Home:
			ScrollToTop();
			break;
		case key_End:
			ScrollToBottom();
			break;
		default:
			if (mView)
				return mView->KeyDown(inInfo);
				
			return false;
	}
	
	return true;
}

void CScrollerView::KeyUp(const SKeyMsgData& inInfo)
{
	switch (inInfo.keyCode)
	{
		case key_PageUp:
		case key_PageDown:
		case key_Home:
		case key_End:
			break;
		default:
			if (mView)
				mView->KeyUp(inInfo);
			break;
	}
}

bool CScrollerView::KeyRepeat(const SKeyMsgData& inInfo)
{
	switch (inInfo.keyCode)
	{
		case key_PageUp:
			Scroll(0, -mContentRect.GetHeight());
			break;
		case key_PageDown:
			Scroll(0, mContentRect.GetHeight());
			break;
		case key_Home:
			ScrollToTop();
			break;
		case key_End:
			ScrollToBottom();
			break;
		default:
			if (mView)
				return mView->KeyRepeat(inInfo);
				
			return false;
	}
	
	return true;
}

void CScrollerView::DragEnter(const SDragMsgData& inInfo)
{
	CView::DragEnter(inInfo);
	CScrollerView::DragMove(inInfo);	
}

void CScrollerView::DragMove(const SDragMsgData& inInfo)
{
	CView::DragMove(inInfo);
	
	if (mView)
	{
		CalcRects();
		
		SDragMsgData info = inInfo;
		info.loc.v -= mDestRect.top;
		info.loc.h -= mDestRect.left;
	
		if (mContentRect.Contains(inInfo.loc))
		{
			if (mView->IsDragWithin())
				mView->DragMove(info);
			else
			{
				mView->DragEnter(info);
				// snuck the following out of DragEnter
				if (mView && !mView->IsSelfDrag(inInfo.drag) && inInfo.drag->GetDragAction())
				{
					mIsDragHilited = true;
					SRect r = mContentRect;
					r.Enlarge(mMargin, mMargin);
					Refresh(r);
				}
			}
		}
		else
		{
			if (mView->IsDragWithin())
				mView->DragLeave(info);
		}
	}
}

void CScrollerView::DragLeave(const SDragMsgData& inInfo)
{
	CView::DragLeave(inInfo);
	
	if (mIsDragHilited)
	{
		mIsDragHilited = false;
		SRect r = mContentRect;
		r.Enlarge(mMargin, mMargin);
		Refresh(r);
	}
	if (mView && mView->IsDragWithin())
	{
		CalcRects();
		
		SDragMsgData info = inInfo;
		info.loc.v -= mDestRect.top;
		info.loc.h -= mDestRect.left;
		mView->DragLeave(info);
	}
}

bool CScrollerView::Drop(const SDragMsgData& inInfo)
{
	if (mView)
	{
		CalcRects();
		
		SDragMsgData info = inInfo;
		info.loc.v -= mDestRect.top;
		info.loc.h -= mDestRect.left;
		return mView->Drop(info);
	}
	return false;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CScrollerView::UpdateQuickTime()
{
	if (mView && mView->IsVisible())		// ignore invisible view
		mView->UpdateQuickTime();
}

void CScrollerView::SendToQuickTime(const EventRecord& inInfo)
{
	if (mView && mView->IsVisible())		// ignore invisible view
		mView->SendToQuickTime(inInfo);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CScrollerView::Timer(TTimer /* inTimer */)
{
	if (IsMouseWithin() && IsAnyMouseBtnDown())
	{
		if (mVScrollPart)
		{
			switch (mVScrollPart)
			{
				case sbPart_Up:
				case sbPart_SecondUp:
					Scroll(0, -32);
					break;
				case sbPart_Down:
				case sbPart_SecondDown:
					Scroll(0, 32);
					break;
				case sbPart_PageUp:
					mScrollTimerWentOff = true;
					Scroll(0, -mContentRect.GetHeight());
					break;
				case sbPart_PageDown:
					mScrollTimerWentOff = true;
					Scroll(0, mContentRect.GetHeight());
					break;
			}
		}
		else if (mHScrollPart)
		{
			switch (mHScrollPart)
			{
				case sbPart_Left:
				case sbPart_SecondLeft:
					Scroll(-32, 0);
					break;
				case sbPart_Right:
				case sbPart_SecondRight:
					Scroll(32, 0);
					break;
				case sbPart_PageLeft:
					mScrollTimerWentOff = true;
					Scroll(-mContentRect.GetHeight(), 0);
					break;
				case sbPart_PageRight:
					mScrollTimerWentOff = true;
					Scroll(mContentRect.GetHeight(), 0);
					break;
			}
		}
	}
}

bool CScrollerView::SetEnable(bool inEnable)
{
	if (mIsEnabled != (inEnable != 0))		// if change
	{
		mIsEnabled = (inEnable != 0);
		if (mView) mView->SetEnable(inEnable);
		UpdateActive();
		
		return true;
	}
	
	return false;
}

void CScrollerView::UpdateActive()
{
	bool drawDisab = IsDisabled() || IsInactive();
	bool drawFocus = !drawDisab && mCanFocus && (mState == viewState_Focus) && (mOptions & scrollerOption_Border) && !(mOptions & scrollerOption_NoFocusBorder);

	if (drawFocus != mIsDrawFocus)
	{
		mIsDrawFocus = drawFocus;
		mIsDrawDisabled = drawDisab;
		Refresh();
	}
	else if (drawDisab != mIsDrawDisabled)
	{
		mIsDrawDisabled = drawDisab;
		
		if ((mOptions & scrollerOption_VertBar) && mVScrollEnabled) Refresh(mVScrollRect);
		if ((mOptions & scrollerOption_HorizBar) && mHScrollEnabled) Refresh(mHScrollRect);
	}
}

bool CScrollerView::ChangeState(Uint16 inState)
{
	if (mState != inState)	// if change
	{
		mState = inState;
		UpdateActive();
		if (mView) mView->ChangeState(inState);
		return true;
	}
	
	return false;
}

bool CScrollerView::TabFocusNext()
{
	if (mView && mView->TabFocusNext() && mState == viewState_Focus) 
		return true;

	return CView::TabFocusNext();
}


bool CScrollerView::TabFocusPrev()
{
	if (mView && mView->TabFocusPrev() && mState == viewState_Focus) 
		return true;

	return CView::TabFocusPrev();
}

bool CScrollerView::SetBounds(const SRect& inBounds)
{
	Int32 oldHeight = mBounds.GetHeight();
	Int32 oldWidth = mBounds.GetWidth();

	if (CView::SetBounds(inBounds))
	{
		SRect r;
		Uint16 sizing;
		Uint32 h, w;

		Int32 newHeight = inBounds.GetHeight();
		Int32 newWidth = inBounds.GetWidth();

		mRectsValid = false;
		mValuesValid = false;
		
		if (mView && (oldHeight != newHeight || oldWidth != newWidth))
		{
			sizing = mView->GetSizing();
			if (sizing)
			{
				mView->GetBounds(r);
				
				if (sizing & sizing_LeftSticky)
					r.left = newWidth - (oldWidth - r.left);
				
				if (sizing & sizing_RightSticky)
					r.right = newWidth - (oldWidth - r.right);
				
				if (sizing & sizing_TopSticky)
					r.top = newHeight - (oldHeight - r.top);
					
				if (sizing & sizing_BottomSticky)
					r.bottom = newHeight - (oldHeight - r.bottom);
				
				if ((sizing & sizing_FullHeight) && (sizing & sizing_FullWidth))
				{
					mView->GetFullSize(w, h);
					r.bottom = r.top + h;
					r.right = r.left + w;
				}
				else if (sizing & sizing_FullHeight)
					r.bottom = r.top + mView->GetFullHeight();
				else if (sizing & sizing_FullWidth)
					r.right = r.left + mView->GetFullWidth();
				
				mView->SetBounds(r);
				
				/*
				 * As a result of changing the bounds, the full height/width may have
				 * changed, so we'll do it again.  For example, when you change the
				 * width of wrapped text, the full height changes too.
				 */
				if ((sizing & sizing_FullHeight) && (sizing & sizing_FullWidth))
				{
					mView->GetFullSize(w, h);
					r.bottom = r.top + h;
					r.right = r.left + w;
				}
				else if (sizing & sizing_FullHeight)
					r.bottom = r.top + mView->GetFullHeight();
				else if (sizing & sizing_FullWidth)
					r.right = r.left + mView->GetFullWidth();
				
				mView->SetBounds(r);
			}
		}
		
		if (mView)
		{
			// if is a QuickTimeView we need to update the movie bounds
			CQuickTimeView *pQuickTimeView = dynamic_cast<CQuickTimeView *>(mView);
			if (pQuickTimeView)
				pQuickTimeView->UpdateQuickTimeBounds();
		}
		
		CalcValuesAndRefresh();
		
		return true;
	}
	
	return false;
}

void CScrollerView::RefreshScrollBars()
{
	CalcRects();
	
	if ((mOptions & scrollerOption_VertBar) && (mOptions & scrollerOption_HorizBar))
	{
		SRect r;
		
		r.top = mVScrollRect.top;
		r.left = mHScrollRect.left;
		r.bottom = mHScrollRect.bottom;
		r.right = mVScrollRect.right;
		
		Refresh(r);
	}
	else if (mOptions & scrollerOption_VertBar)
		Refresh(mVScrollRect);
	else if (mOptions & scrollerOption_HorizBar)
		Refresh(mHScrollRect);
}

void CScrollerView::MouseCaptureReleased()
{
	CView::MouseCaptureReleased();
	
	if (mMouseCapture)
	{
		if (mView) mView->MouseCaptureReleased();
		mMouseCapture = false;
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CScrollerView::CalcRects()
{
	if (!mRectsValid)
	{
		IntCalcRects();
		mRectsValid = true;
	}
}

void CScrollerView::RecalcRects()
{
	mRectsValid = false;
}

void CScrollerView::IntCalcRects()
{
	const Int16 barThick = 16;
	const Int16 growSpace = 14;
	Int16 border, barShift, margin;
	Int32 destHorizDelta, destVertDelta;
	SRect r;
	
	destVertDelta = mContentRect.top - mDestRect.top;
	destHorizDelta = mContentRect.left - mDestRect.left;
	
	if (mOptions & scrollerOption_Border)
	{
		border = 2;
		barShift = 1;
	}
	else if (mOptions & scrollerOption_PlainBorder)
	{
		border = 1;
		barShift = 1;
	}
	else
	{
		border = 0;
		barShift = 0;
	}
	
	if (mCanFocus && (!(mOptions & scrollerOption_NoFocusBorder)))
		border++;
	
	margin = mMargin;

	if ((mOptions & scrollerOption_VertBar) && (mOptions & scrollerOption_HorizBar))
	{
		mContentRect.left = mBounds.left + border + margin;
		mContentRect.top = mBounds.top + border + margin;
		mContentRect.right = mBounds.right - border - margin - barThick + barShift;
		mContentRect.bottom = mBounds.bottom - border - margin - barThick + barShift;
	
		mVScrollRect.left = mBounds.right - border - barThick + barShift;
		mVScrollRect.top = mBounds.top + border - barShift;
		mVScrollRect.right = mBounds.right - border + barShift;
		mVScrollRect.bottom = mBounds.bottom - border - growSpace;
	
		mHScrollRect.left = mBounds.left + border - barShift;
		mHScrollRect.top = mBounds.bottom - border - barThick + barShift;
		mHScrollRect.right = mBounds.right - border - growSpace;
		mHScrollRect.bottom = mBounds.bottom - border + barShift;
	}
	else if (mOptions & scrollerOption_VertBar)
	{
		mContentRect.left = mBounds.left + border + margin;
		mContentRect.top = mBounds.top + border + margin;
		mContentRect.right = mBounds.right - border - margin - barThick + barShift;
		mContentRect.bottom = mBounds.bottom - border - margin;
	
		mVScrollRect.left = mBounds.right - border - barThick + barShift;
		mVScrollRect.top = mBounds.top + border - barShift;
		mVScrollRect.right = mBounds.right - border + barShift;
		
#if MACINTOSH
		if (mOptions & scrollerOption_GrowSpace)
			mVScrollRect.bottom = mBounds.bottom - border - growSpace;
		else
#endif
			mVScrollRect.bottom = mBounds.bottom - border + barShift;
	}
	else if (mOptions & scrollerOption_HorizBar)
	{
		mContentRect.left = mBounds.left + border + margin;
		mContentRect.top = mBounds.top + border + margin;
		mContentRect.right = mBounds.right - border - margin;
		mContentRect.bottom = mBounds.bottom - border - margin - barThick + barShift;
	
		mHScrollRect.left = mBounds.left + border - barShift;
		mHScrollRect.top = mBounds.bottom - border - barThick + barShift;
		mHScrollRect.bottom = mBounds.bottom - border + barShift;

#if MACINTOSH
		if (mOptions & scrollerOption_GrowSpace)
			mHScrollRect.right = mBounds.right - border - growSpace;
		else
#endif
			mHScrollRect.right = mBounds.right - border + barShift;
	}
	else
	{
		mContentRect = mBounds;
		mContentRect.Enlarge(-(border+margin),-(border+margin));
	}
	
	if (mDestRect.top == 0 && mDestRect.left == 0 && mDestRect.bottom == 0 && mDestRect.right == 0)
	{
		mDestRect.top = mContentRect.top;
		mDestRect.left = mContentRect.left;
	}
	else
	{
		mDestRect.top = mContentRect.top - destVertDelta;
		mDestRect.left = mContentRect.left - destHorizDelta;
	}
	
	if (mView)
	{
		mView->GetBounds(r);
		mDestRect.bottom = mDestRect.top + r.GetHeight();
		mDestRect.right = mDestRect.left + r.GetWidth();
	}
	else
	{
		mDestRect.bottom = mDestRect.top;
		mDestRect.right = mDestRect.left;
	}
}

void CScrollerView::CalcValues()
{
	if (!mValuesValid)
	{
		CalcRects();		// values rely on rects
		IntCalcValues();
		mValuesValid = true;
		
		if (!mVScrollEnabled) ScrollToTop();
		if (!mHScrollEnabled) ScrollToLeft();
	}
}

void CScrollerView::CalcValuesAndRefresh()
{
	if (!mValuesValid)
	{
		Uint32 saveHScrollVal = mHScrollVal;
		Uint32 saveVScrollVal = mVScrollVal;
		Uint32 saveHScrollMax = mHScrollMax;
		Uint32 saveVScrollMax = mVScrollMax;
		Uint8 saveHScrollEnabled = mHScrollEnabled;
		Uint8 saveVScrollEnabled = mVScrollEnabled;
	
		CalcRects();
		IntCalcValues();
		mValuesValid = true;
		
		SRect updateRect(0,0,0,0);

		if (saveHScrollVal != mHScrollVal || saveHScrollMax != mHScrollMax || saveHScrollEnabled != mHScrollEnabled)
			updateRect |= mHScrollRect;
		
		if (saveVScrollVal != mVScrollVal || saveVScrollMax != mVScrollMax || saveVScrollEnabled != mVScrollEnabled)
			updateRect |= mVScrollRect;
		
		Refresh(updateRect);

		if (mDestRect.bottom < mContentRect.bottom)
			ScrollToBottom();
		if (mDestRect.right < mContentRect.right)
			ScrollToRight();
	}
}

void CScrollerView::RecalcValues()
{
	mValuesValid = false;
}

void CScrollerView::IntCalcValues()
{
	Uint32 destHeight = mDestRect.GetHeight();
	Uint32 contentHeight = mContentRect.GetHeight();

	mVScrollMax = destHeight - contentHeight;
	mVScrollVal = mContentRect.top - mDestRect.top;
	mVScrollEnabled = destHeight > contentHeight;

	Uint32 destWidth = mDestRect.GetWidth();
	Uint32 contentWidth = mContentRect.GetWidth();

	mHScrollMax = destWidth - contentWidth;
	mHScrollVal = mContentRect.left - mDestRect.left;
	mHScrollEnabled = destWidth > contentWidth;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CScrollerView::HandleInstall(CView *inView)
{
	ASSERT(inView);
	
	if (mView != inView)
	{
		inView->ChangeState(mState);
		
		CSingleViewContainer::HandleInstall(inView);
		
		mRectsValid = mValuesValid = false;
		Refresh();
	}
}

void CScrollerView::HandleRemove(CView *inView)
{
	if (mView == inView)
	{
		CSingleViewContainer::HandleRemove(inView);
		
		mRectsValid = mValuesValid = false;
		mMouseCapture = false;
		Refresh();
	}
}

void CScrollerView::HandleRefresh(CView */* inView */, const SRect& inUpdateRect)
{
	CalcRects();
	SRect r = inUpdateRect;
	r.Move(mDestRect.left, mDestRect.top);
	r.Constrain(mContentRect);
	Refresh(r);
}

void CScrollerView::HandleHit(CView *inView, const SHitMsgData& inInfo)
{
	if (mHandler) mHandler->HandleHit(inView, inInfo);
}

void CScrollerView::HandleGetScreenDelta(CView */* inView */, Int32& outHoriz, Int32& outVert)
{
	GetScreenDelta(outHoriz, outVert);
	
	outHoriz += mDestRect.left;
	outVert += mDestRect.top;
}

void CScrollerView::HandleGetVisibleRect(CView */* inView */, SRect& outRect)
{		
	SRect stVisibleRect;
	GetVisibleRect(stVisibleRect);

	CalcRects();
	SRect stContentRect = mContentRect;
	
	stVisibleRect.GetIntersection(stContentRect, outRect);
	outRect.Move(-mDestRect.left, -mDestRect.top);
}

void CScrollerView::HandleSetBounds(CView */* inView */, const SRect& /* inBounds */)
{
	mRectsValid = mValuesValid = false;
	CalcValuesAndRefresh();
}

void CScrollerView::HandleMakeRectVisible(const SRect& inRect, Uint16 inAlign)
{
	ScrollToRect(inRect, inAlign);
}

void CScrollerView::HandleCaptureMouse(CView *inView)
{
	if (!mMouseCapture && mView == inView)
	{
		mMouseCapture = true;
		if (mHandler) mHandler->HandleCaptureMouse(this);
	}
}

void CScrollerView::HandleReleaseMouse(CView *inView)
{
	if (mMouseCapture && mView == inView)
	{	if (mHandler) mHandler->HandleReleaseMouse(this);	}
}

void CScrollerView::HandleGetOrigin(SPoint& outOrigin)
{
	if (mHandler) mHandler->HandleGetOrigin(outOrigin);

	outOrigin.x += mDestRect.left;
	outOrigin.y += mDestRect.top;
}

CView *CScrollerView::HandleGetCaptureMouseView()
{
	if (mMouseCapture)
		return mView;

	return nil;
}

CWindow *CScrollerView::HandleGetParentWindow()
{
	return GetParentWindow();
}

