/*****************************************************************************

Possible change of CView::MouseMove() behaviour:

Currently, MouseMove() is not called unless the mouse is within the view
and MouseEnter() has been called.  Once MouseLeave() has been called,
MouseMove() is not called again.

However, when the mouse is down, some views would like to keep tracking the
mouse even after it has left.  For example, CTextView.

Thus, if we changed CView to allow this:

	- MouseMove() can be called at any time (regardless of whether MouseEnter()
	  has been called).

	- The caller will *usually* only call MoveMouse() without MouseEnter()
	  when the mouse went down in the view.

*****************************************************************************/

/************************************************************************

BETTER REFRESH MECHANISM >>>> keep multiple rectangles that need to be refreshed,
and draw each rectangle separately.   Merge overlapping rectangles.
Merge NEAR rectangles??

ALSO, allow a "kMore" style flag to be set >>> indicates that Refresh() will
be called again very soon and don't draw until it's called without kMore.
Good for if the drawing is in a thread (eg, BeOS).

**************************************************************************/


#include "CView.h"
#include "UKeyboard.h"

/*
 * Structures
 */

#pragma options align=packed

typedef struct {
	SRect bounds;
	Uint32 id;
	Uint32 commandID;
	Uint16 sizing;
	Uint16 isVisible	: 1;
	Uint16 isActive		: 1;
	Uint16 isEnabled	: 1;
	Uint16 isFocus		: 1;
	Uint16 canFocus		: 1;
	Uint32 rsvd;
} SView;

#pragma options align=reset

/* -------------------------------------------------------------------------- */

CView::CView(CViewHandler *inHandler, const SRect& inBounds)
{
#if DEBUG
	if (inBounds.IsInvalid()) DebugBreak("bounds rect for view is invalid");
#endif

	mBounds = inBounds;
	mID = mCommandID = 0;
	mSizing = 0;
	mState = viewState_Hidden;

	mIsLeftMouseBtnDown = mIsRightMouseBtnDown = mIsMiddleMouseBtnDown = mIsMouseWithin = mIsDragWithin = mIsDragHilited = mIsVisible = mCanFocus = mHasChanged = mHasMouseCapture = false;
	mIsEnabled = true;
	
	mHandler = nil;
	if (inHandler)
	{
		inHandler->HandleInstall(this);
		mHandler = inHandler;
	}
	
	mTooltipMsg1 = nil;
	mTooltipMsg2 = nil;
}

CView::~CView()
{
	UApplication::FlushMessages(TimerHandler, this);

	if (mHandler)
	{
		if (mHasMouseCapture)
			mHandler->HandleReleaseMouse(this);
		
		mHandler->HandleRemove(this);
	}
		
	if (mTooltipMsg1)
		UMemory::Dispose(mTooltipMsg1);
	
	if (mTooltipMsg2)
		UMemory::Dispose(mTooltipMsg2);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CView::SetHandler(CViewHandler *inHandler)
{
	if (mHandler != inHandler)							// if change
	{
		// remove from current handler
		if (mHandler)
		{
			Refresh();
			mHandler->HandleRemove(this);
			mHandler = nil;
		}
		
		// install in new handler
		if (inHandler)
		{
			inHandler->HandleInstall(this);
			mHandler = inHandler;
			Refresh();
		}
	}
}

void CView::GetRegion(TRegion outRgn) const
{
	URegion::SetRect(outRgn, mBounds);
}

// returns true if changed
bool CView::SetBounds(const SRect& inBounds)
{
	if (mBounds != inBounds)
	{
#if DEBUG
		if (inBounds.IsInvalid()) DebugBreak("CView::SetBounds - invalid rect");
#endif

		SRect saveBounds = mBounds;
		
		try
		{
			SRect updateRect = saveBounds;
			updateRect |= inBounds;
			
			mBounds = inBounds;
			Refresh(updateRect);
			
			if (mHandler)
				mHandler->HandleSetBounds(this, inBounds);
		}
		catch(...)
		{
			mBounds = saveBounds;
			throw;
		}
		
		return true;
	}
	
	return false;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

// note that viewState_Hidden is not related to the visibility of the view (eg a view can be visible but hidden, such if the window the view is in is hidden)
// returns true if changed
bool CView::SetVisible(bool inVisible)
{
	if (mIsVisible != (inVisible!=0))		// if change
	{
		bool saveVisible = mIsVisible;
		
		try
		{
			mIsVisible = true;
			Refresh();
			
			if (mHandler) mHandler->HandleSetVisible(this, inVisible);
		}
		catch(...)
		{
			mIsVisible = (saveVisible != 0);
			throw;
		}
		
		mIsVisible = (inVisible != 0);
		
		return true;
	}
	
	return false;
}

// returns true if changed
bool CView::SetEnable(bool inEnable)
{
	if (mIsEnabled != (inEnable != 0))		// if change
	{
		mIsEnabled = (inEnable != 0);
		return true;
	}
	
	return false;
}

// this does need to be a virtual function as CScrollerView overrides it
void CView::SetCanFocus(bool inCanFocus)
{
	mCanFocus = (inCanFocus != 0);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

// returns true if changed
bool CView::ChangeState(Uint16 inState)
{
	if (mState != inState)	// if change
	{
		mState = inState;
		Uint16 nTooltipState = UTooltip::GetState();
		
		if ((inState == viewState_Hidden || inState == viewState_Inactive) && (nTooltipState == kTooltipState_ActiveVisible || nTooltipState == kTooltipState_Activating))
			ReleaseMouse();

		return true;
	}
	
	return false;
}

// returns true if something was focused, false if unfocused
bool CView::TabFocusNext()
{
	if(!(mCanFocus && mIsVisible && mIsEnabled))
		return false;

	if(mState == viewState_Active)
	{
		ChangeState(viewState_Focus);
		return true;
	}
	
	if(mState == viewState_Focus)
		ChangeState(viewState_Active);

	return false;
}

// returns true if something was focused, false if unfocused
bool CView::TabFocusPrev()
{
	if(!(mCanFocus && mIsVisible && mIsEnabled))
		return false;
	
	if(mState == viewState_Active)
	{
		ChangeState(viewState_Focus);
		return true;
	}
	
	if(mState == viewState_Focus)
		ChangeState(viewState_Active);

	return false;
}


void CView::Draw(TImage /* inImage */, const SRect& /* inUpdateRect */, Uint32 /* inDepth */)
{
	// derived classes override this to draw
}

void CView::Hit(Uint32 inType, Uint32 inPart, Uint32 inParam, const void *inData, Uint32 inDataSize)
{
	if (mHandler)
	{
	#pragma options align=packed
		struct {
			SHitMsgData hit;
			Uint8 data[256];
		} info;
	#pragma options align=reset
	
		if (inDataSize > sizeof(info.data))
		{
			DebugBreak("CView - hit data cannot exceed 256 bytes");
			Fail(errorType_Misc, error_Param);
		}
		
		info.hit.view = this;
		info.hit.id = mID;
		info.hit.cmd = mCommandID;
		info.hit.type = inType;
		info.hit.part = inPart;
		info.hit.param = inParam;
		info.hit.dataSize = inDataSize;
		
		if (inData && inDataSize)
			UMemory::Copy(info.data, inData, inDataSize);
		
		mHandler->HandleHit(this, info.hit);
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

/*
 * Call MouseDown() to indicate that a button on the mouse has gone down while the
 * pointer/cursor was located within the view.  Note that MouseEnter() should be
 * called before MouseDown() - the mouse should always enter the view before going
 * down.
 */
void CView::MouseDown(const SMouseMsgData& inInfo)
{
	switch (inInfo.button)
	{
		case mouseButton_Left:
			mIsLeftMouseBtnDown = true;
			break;
		case mouseButton_Right:
			mIsRightMouseBtnDown = true;
			break;
		case mouseButton_Middle:
			mIsMiddleMouseBtnDown = true;
			break;
	}
}

void CView::MouseUp(const SMouseMsgData& inInfo)
{
	switch (inInfo.button)
	{
		case mouseButton_Left:
			mIsLeftMouseBtnDown = false;
			break;
		case mouseButton_Right:
			mIsRightMouseBtnDown = false;
			break;
		case mouseButton_Middle:
			mIsMiddleMouseBtnDown = false;
			break;
	}
	
	Uint16 nTooltipState = UTooltip::GetState();
	
	if (nTooltipState == kTooltipState_ActiveVisible || nTooltipState == kTooltipState_Activating)
	{
		UTooltip::Hide();
		ReleaseMouse();	
	}
}

void CView::MouseEnter(const SMouseMsgData& inInfo)
{
#if DEBUG
	if (mIsMouseWithin)
	{
		DebugBreak("CView - MouseEnter() called but mouse has already entered");
		return;
	}
#endif

	mIsMouseWithin = true;
	
	if (UTooltip::IsEnabled())
	{
		if (inInfo.mods & modKey_Control)
		{
			if (mTooltipMsg2)
			{
				CaptureMouse();
				UTooltip::Activate(this, (Uint8 *)mTooltipMsg2);
			}
		}
		else if (mTooltipMsg1)
		{
			CaptureMouse();
			UTooltip::Activate(this, (Uint8 *)mTooltipMsg1);
		}
	}
}

void CView::MouseMove(const SMouseMsgData& inInfo)
{
#if DEBUG
//	if (!mIsMouseWithin)
//		DebugBreak("CView - mouse should enter view before moving (MouseEnter() before MouseMove())");
#endif

	if (!mBounds.Contains(inInfo.loc))
	{
		Uint16 nTooltipState = UTooltip::GetState();

		if (nTooltipState == kTooltipState_ActiveVisible || nTooltipState == kTooltipState_Activating)
		{
			UTooltip::Hide();
			ReleaseMouse();
		}
	}
}

/*
 * MouseLeave() is called when the pointer/cursor has moved outside of the view,
 * and must be called after MouseEnter().  If the mouse is down, views may want
 * to start a drag from their IntMouseLeave() function.
 */
void CView::MouseLeave(const SMouseMsgData& /* inInfo */)
{
#if DEBUG
	if (!mIsMouseWithin)
	{
		DebugBreak("CView - MouseLeave() called without matching MouseEnter()");
		return;
	}
#endif

	mIsMouseWithin = false;

	Uint16 nTooltipState = UTooltip::GetState();

	if (nTooltipState == kTooltipState_ActiveVisible || nTooltipState == kTooltipState_Activating)
	{
		UTooltip::Hide();
		ReleaseMouse();
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CView::CaptureMouse()
{
	if (mHandler)
		mHandler->HandleCaptureMouse(this);
	
	mHasMouseCapture = true;
}

void CView::ReleaseMouse()
{
	if (mHandler)
		mHandler->HandleReleaseMouse(this);
	
	mHasMouseCapture = false;
}

void CView::MouseCaptureReleased()
{
	Uint16 nTooltipState = UTooltip::GetState();

	if (nTooltipState == kTooltipState_ActiveVisible || nTooltipState == kTooltipState_Activating)
		UTooltip::Hide();
}

/* -------------------------------------------------------------------------- */
#pragma mark -

bool CView::KeyDown(const SKeyMsgData& /* inInfo */)
{
	// derived classes override this function
	
	return false;
}

void CView::KeyUp(const SKeyMsgData& /* inInfo */)
{
	// derived classes override this function
}

bool CView::KeyRepeat(const SKeyMsgData& /* inInfo */)
{
	// derived classes override this function
	
	return false;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

/*
 * CanAcceptDrop() returns whether or not the view can accept a drop of the
 * specified drag.  However, just because a views says it *can* accept a 
 * drop, doesn't mean it *will* accept the drop.
 */
 
// TG - change of plan.  I just call drag enter on the view.  if it can accept a drop, it calls
// SetDragAction() with the type of drag that will be done.  this way the drag knows what's about to happen and the cursor can be updated
// also, this fixes the problem where a container will return true to CanAccpetDrop even if no view in that location will take the drop.
#if 0
bool CView::CanAcceptDrop(TDrag /* inDrag */) const
{
	// by default, the view can't accept any drop
	return false;
}
#endif
/*
 * IsSelfDrag() returns whether or not the specified drag was initiated
 * by this view.
 */
bool CView::IsSelfDrag(TDrag /* inDrag */) const
{
	// by default, assume is not a drag from this view
	return false;
}

void CView::DragEnter(const SDragMsgData& inInfo)
{
#if DEBUG
	if (mIsDragWithin)
		DebugBreak("CView - DragEnter() called but drag has already entered");
#endif
	// by default there is no action
	// there's a bit of overhead in doing this, but it must be done since by default there is no drop
	// also, there could be bugs where CView::DragEnter is called after seting the drag action in the derived view
	inInfo.drag->SetDragAction(dragAction_None);	
	mIsDragWithin = true;
}

void CView::DragMove(const SDragMsgData& inInfo)
{
	// derived classes override this function
	// by default there is no action
	inInfo.drag->SetDragAction(dragAction_None);	
}

void CView::DragLeave(const SDragMsgData& /* inInfo */)
{
#if DEBUG
	if (!mIsDragWithin)
	{
		DebugBreak("CView - DragLeave() called without matching DragEnter()");
		return;
	}
#endif

	mIsDragWithin = false;
}

// returns whether or not drag was accepted
bool CView::Drop(const SDragMsgData& /* inInfo */)
{
	// don't accept by default
	return false;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CView::UpdateQuickTime()
{
	// some derived classes override this function
}

void CView::SendToQuickTime(const EventRecord& /* inInfo */)
{
	// some derived classes override this function
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CView::TimerHandler(void *inContext, void *inObject, Uint32 /* inMsg */, const void */* inData */, Uint32 /* inDataSize */)
{
	((CView *)inContext)->Timer((TTimer)inObject);
}

void CView::Timer(TTimer /* inTimer */)
{
	// by default, do nothing if a timer went off
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CView::GetFullSize(Uint32& outWidth, Uint32& outHeight) const
{
	outHeight = GetFullHeight();
	outWidth = GetFullWidth();
}

Uint32 CView::GetFullHeight() const
{
	return mBounds.GetHeight();
}

Uint32 CView::GetFullWidth() const
{
	return mBounds.GetWidth();
}

void CView::SetFullSize()
{
	Uint32 h, w;
	SRect r;
	
	GetFullSize(w, h);
	
	r.top = mBounds.top;
	r.left = mBounds.left;
	r.bottom = r.top + h;
	r.right = r.left + w;
	
	SetBounds(r);
}

void CView::SetFullHeight()
{
	SRect r;
	
	r.top = mBounds.top;
	r.left = mBounds.left;
	r.bottom = r.top + GetFullHeight();
	r.right = mBounds.right;
	
	SetBounds(r);
}

void CView::SetFullWidth()
{
	SRect r;
	
	r.top = mBounds.top;
	r.left = mBounds.left;
	r.bottom = mBounds.bottom;
	r.right = r.left + GetFullWidth();
	
	SetBounds(r);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void CView::Refresh(const SRect& inRect)
{
	if (mHandler && mIsVisible && mState != viewState_Hidden)
	{
		SRect r = inRect;
		r.Constrain(mBounds);
		
		if (!r.IsEmpty())
			mHandler->HandleRefresh(this, r);
	}
}

void CView::GetScreenDelta(Int32& outHoriz, Int32& outVert) const
{
	if (mHandler)
		mHandler->HandleGetScreenDelta((CView *)this, outHoriz, outVert);
	else
		outHoriz = outVert = 0;
}

bool CView::IsPointWithin(const SPoint& inPt) const
{
	return mBounds.Contains(inPt);
}

void CView::SetHasChanged(bool inHasChanged)
{
	mHasChanged = (inHasChanged != 0);
}

void CView::MakeRectVisible(const SRect& inRect, Uint16 inAlign)
{
	if (mHandler) 
		mHandler->HandleMakeRectVisible(inRect, inAlign);
}

void CView::GetVisibleRect(SRect& outRect) const
{
	if (mHandler)
	{
		SRect stVisibleRect;
		mHandler->HandleGetVisibleRect((CView *)this, stVisibleRect);
		
		stVisibleRect.GetIntersection(mBounds, outRect);
	}
	else
		outRect = mBounds;
}

void CView::SetTooltipMsg(const Uint8 inMsg1[], const Uint8 inMsg2[])
{
	if (mTooltipMsg1)
	{
		UMemory::Dispose(mTooltipMsg1);
		mTooltipMsg1 = nil;
	}
	
	if (mTooltipMsg2)
	{
		UMemory::Dispose(mTooltipMsg2);
		mTooltipMsg2 = nil;
	}

	if (inMsg1 && inMsg1[0])
		mTooltipMsg1 = UMemory::New(inMsg1, inMsg1[0] + 1);
		
	if (inMsg2 && inMsg2[0])
		mTooltipMsg2 = UMemory::New(inMsg2, inMsg2[0] + 1);
}

CWindow *CView::GetParentWindow()
{
	return mHandler ? mHandler->HandleGetParentWindow() : nil;
}


/* -------------------------------------------------------------------------- */
#pragma mark -

void CViewHandler::HandleInstall(CView *) {}
void CViewHandler::HandleRemove(CView *) {}
void CViewHandler::HandleSetBounds(CView *, const SRect&) {}
void CViewHandler::HandleRefresh(CView *, const SRect&) {}
void CViewHandler::HandleSetVisible(CView *, bool) {}
void CViewHandler::HandleHit(CView *, const SHitMsgData&) {}
void CViewHandler::HandleMakeRectVisible(const SRect&, Uint16) {}
void CViewHandler::HandleCaptureMouse(CView *)	{}
void CViewHandler::HandleReleaseMouse(CView *)	{}
CView *CViewHandler::HandleGetCaptureMouseView()	{ return nil;	}
CWindow *CViewHandler::HandleGetParentWindow()		{ return nil;	}


void CViewHandler::HandleGetScreenDelta(CView */* inView */, Int32& outHoriz, Int32& outVert)
{
	outHoriz = outVert = 0;
}

void CViewHandler::HandleGetVisibleRect(CView *inView, SRect& outRect)
{
	if (inView)
		inView->GetBounds(outRect);
	else
		outRect.SetEmpty();
}

void CViewHandler::HandleGetOrigin(SPoint& outOrigin)
{
	outOrigin.x = 0;
	outOrigin.y = 0;
}

