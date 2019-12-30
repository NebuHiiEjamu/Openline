/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "CView.h"
#include "CViewContainer.h"

enum {
	scrollerOption_HorizBar			= 0x01,
	scrollerOption_VertBar			= 0x02,
	scrollerOption_GrowSpace		= 0x04,
	scrollerOption_Border			= 0x08,
	scrollerOption_PlainBorder		= 0x10,
	scrollerOption_NoFocusBorder	= 0x20,
	scrollerOption_NoBkgnd			= 0x40
};

class CScrollerView : public CView, public CSingleViewContainer
{	
	public:
		// construction
		CScrollerView(CViewHandler *inHandler, const SRect& inBounds);
		CScrollerView(CViewHandler *inHandler, const SRect& inBounds, Uint32 inOptions, Uint16 inMargin = 0);
		virtual ~CScrollerView();
		
		// options
		virtual void SetOptions(Uint16 inOptions);
		Uint16 GetOptions() const;
		virtual void SetCanFocus(bool inCanFocus);
		virtual void SetMargin(Uint16 inSpace);
		Uint16 GetMargin() const;
		
		// content
		Int32 GetVisibleContentHeight() const;
		Int32 GetVisibleContentWidth() const;
		void SetContentColor(const SColor& inColor);
		void GetContentColor(SColor& outColor);
		
		// scrolling
		virtual void SetScroll(Uint32 inHorizPos, Uint32 inVertPos);
		void GetScroll(Uint32& outHorizPos, Uint32& outVertPos) const;
		void Scroll(Int32 inHorizDelta, Int32 inVertDelta);
		void ScrollToTop();
		void ScrollToLeft();
		void ScrollToBottom();
		void ScrollToRight();
		void ScrollToRect(const SRect& inRect, Uint16 inAlign = align_Inside);
		bool IsScrolledToBottom();
		
		// misc
		virtual void Timer(TTimer inTimer);
		virtual bool ChangeState(Uint16 inState);
		virtual bool TabFocusNext();
		virtual bool TabFocusPrev();
		virtual bool SetEnable(bool inEnable);
		virtual bool SetBounds(const SRect& inBounds);
		virtual void Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth);
		virtual void MouseCaptureReleased();
		
		// events
		virtual void MouseDown(const SMouseMsgData& inInfo);
		virtual void MouseUp(const SMouseMsgData& inInfo);
		virtual void MouseEnter(const SMouseMsgData& inInfo);
		virtual void MouseMove(const SMouseMsgData& inInfo);
		virtual void MouseLeave(const SMouseMsgData& inInfo);
		virtual bool KeyDown(const SKeyMsgData& inInfo);
		virtual void KeyUp(const SKeyMsgData& inInfo);
		virtual bool KeyRepeat(const SKeyMsgData& inInfo);
		virtual void DragEnter(const SDragMsgData& inInfo);
		virtual void DragMove(const SDragMsgData& inInfo);
		virtual void DragLeave(const SDragMsgData& inInfo);
		virtual bool Drop(const SDragMsgData& inInfo);

		// QuickTime events
		virtual void UpdateQuickTime();
		virtual void SendToQuickTime(const EventRecord& inInfo);

	protected:
		SRect mDestRect, mContentRect;
		SRect mHScrollRect, mVScrollRect;
		Uint32 mHScrollVal, mVScrollVal;
		Uint32 mHScrollMax, mVScrollMax;
		Uint16 mHScrollPart, mVScrollPart;
		Int32 mThumbDelta;
		TTimer mScrollTimer;
		Uint16 mOptions;
		Uint16 mMargin;
		Uint16 mRectsValid			: 1;
		Uint16 mValuesValid			: 1;
		Uint16 mHScrollEnabled		: 1;
		Uint16 mVScrollEnabled		: 1;
		Uint16 mScrollTimerWentOff	: 1;
		Uint16 mIsDrawDisabled		: 1;
		Uint16 mIsDrawFocus			: 1;
		Uint16 mMouseCapture		: 1;
		SColor mContentColor;
			
		// rects
		void CalcRects();
		void RecalcRects();
		virtual void IntCalcRects();
		
		// values
		void CalcValues();
		void CalcValuesAndRefresh();
		void RecalcValues();
		virtual void IntCalcValues();
		
		// misc
		void UpdateActive();
		void RefreshScrollBars();
		
		// handler callbacks
		virtual void HandleInstall(CView *inView);
		virtual void HandleRemove(CView *inView);
		virtual void HandleRefresh(CView *inView, const SRect& inUpdateRect);
		virtual void HandleHit(CView *inView, const SHitMsgData& inInfo);
		virtual void HandleGetScreenDelta(CView *inView, Int32& outHoriz, Int32& outVert);
		virtual void HandleGetVisibleRect(CView *inView, SRect& outRect);
		virtual void HandleSetBounds(CView *inView, const SRect& inBounds);
		virtual void HandleMakeRectVisible(const SRect& inRect, Uint16 inAlign);
		virtual void HandleCaptureMouse(CView *inView);
		virtual void HandleReleaseMouse(CView *inView);
		virtual void HandleGetOrigin(SPoint& outOrigin);
		virtual CView *HandleGetCaptureMouseView();
		virtual CWindow *HandleGetParentWindow();
};

