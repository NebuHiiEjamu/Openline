/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "CView.h"
#include "CViewContainer.h"

class CTabbedView : public CView, public CMultiViewContainer
{
	public:
		// construction
		CTabbedView(CViewHandler *inHandler, const SRect& inBounds);
		virtual ~CTabbedView();
		
		// add and remove tabs
		Uint16 GetTabCount() const;
		void InsertTab(Uint16 inBeforeTab, const void *inText, Uint32 inSize);
		void AddTab(const Uint8 inTitle[]);
		void RemoveTab(Uint16 inTab);
		bool IsValidTab(Uint16 inTab) const;
		
		// tab properties
		void SetTabTitle(Uint16 inTab, const void *inText, Uint32 inSize);
		void SetTabTitle(Uint16 inTab, const Uint8 inText[]);
		Uint32 GetTabTitle(Uint16 inTab, void *outText, Uint32 inMaxSize) const;
		void SetFont(TFontDesc inFont);
		void SetTabView(Uint16 inTab, CView *inView);
		CView *GetTabView(Uint16 inTab) const;
		CView *DetachTabView(Uint16 inTab);
		void SetTabHeight(Uint16 inHeight);
		Uint16 GetTabHeight() const;
		Uint16 GetTotalTabWidth() const;
		
		// current tab
		void SetCurrentTab(Uint16 inTab);
		Uint16 GetCurrentTab() const;
				
		// misc
		void GetContentRect(SRect& outRect) const;
//		virtual bool CanAcceptDrop(TDrag inDrag) const;
		virtual bool ChangeState(Uint16 inState);
		virtual bool SetEnable(bool inEnable);
		virtual bool SetBounds(const SRect& inBounds);
		virtual void Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth);
		virtual void MouseCaptureReleased();

		// events
		virtual void MouseDown(const SMouseMsgData& inInfo);
		virtual void MouseUp(const SMouseMsgData& inInfo);
		virtual void MouseEnter(const SMouseMsgData& inInfo);
		virtual void MouseLeave(const SMouseMsgData& inInfo);
		virtual void MouseMove(const SMouseMsgData& inInfo);
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

		virtual bool TabFocusNext();
		virtual bool TabFocusPrev();

	protected:
		CView *mFocusView;
		THdl mTabInfo;
		TFontDesc mFont;
		Int16 mTextScript;
		Uint16 mTabCount;
		Uint16 mCurrentTab;
		Uint16 mTabHeight;
		Uint16 mHilitedTab;
		Uint16 mMouseCaptureTab;
		CView *mMouseCaptureView;
//		Uint8 mDragWithinViewCanAccept	: 1;
		Uint8 mMouseDownInTabs			: 1;
		Uint8 mIsDrawDisabled			: 1;
		
		// drawing
		virtual void DrawTab(TImage inImage, const SRect& inRect, const void *inTitleData, Uint32 inTitleLen, const SColor& inFrameColor, const SColor& inContentColor, const SColor& inLightColor, const SColor& inDarkColor, const SColor& inTitleColor);
		virtual void DrawEnabledTab(TImage inImage, const SRect& inRect, const void *inTitleData, Uint32 inTitleLen, bool inCurrent);
		virtual void DrawDisabledTab(TImage inImage, const SRect& inRect, const void *inTitleData, Uint32 inTitleLen, bool inCurrent);

		// misc
		void UpdateActive();
		void RefreshContent();
		void RefreshTabs(Uint16 inStartTab = 1, Uint16 inEndTab = max_Uint16);
		void RefreshTab(Uint16 inTab);
		virtual Uint16 GetTabWidth(const void *inText, Uint32 inSize) const;
		virtual Int32 GetTabLeft(Uint16 inTab) const;
		virtual Int32 GetTabRight(Uint16 inTab) const;
		virtual void GetTabRect(Uint16 inTab, SRect& outRect) const;
		virtual Uint16 PointToTab(const SPoint& inPt, SRect *outTabRect = nil) const;
		
		// view handler callbacks
		virtual void HandleRemove(CView *inView);
		virtual void HandleRefresh(CView *inView, const SRect& inUpdateRect);
		virtual void HandleHit(CView *inView, const SHitMsgData& inInfo);
		virtual void HandleGetScreenDelta(CView *inView, Int32& outHoriz, Int32& outVert);
		virtual void HandleGetVisibleRect(CView *inView, SRect& outRect);
		virtual void HandleCaptureMouse(CView *inView);
		virtual void HandleReleaseMouse(CView *inView);
		virtual void HandleGetOrigin(SPoint& outOrigin);
		virtual CView *HandleGetCaptureMouseView();
		virtual CWindow *HandleGetParentWindow();
};

