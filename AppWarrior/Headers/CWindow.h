/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "UWindow.h"
#include "CView.h"
#include "CViewContainer.h"
#include "CPtrList.h"
#include "CLinkedList.h"

enum {
	hitType_WindowCloseBox = 20
};

class CWindow : public CSingleViewContainer
{	
	public:
		enum { ClassID = 'wind' };
		
	public:
		// construction
		CWindow(const SRect& inBounds, Uint16 inLayer = windowLayer_Standard, Uint16 inOptions = 0, Uint16 inStyle = 0, CWindow *inParent = nil);
		virtual ~CWindow();
		
		// title
		void SetTitle(const Uint8 *inTitle)												{	UWindow::SetTitle(mWindow, inTitle);									}
		void SetNoTitle()																{	UWindow::SetNoTitle(mWindow);											}
		Uint32 GetTitle(void *outText, Uint32 inMaxSize) const							{	return UWindow::GetTitle(mWindow, outText, inMaxSize);					}
		
		// visibility
		void SetVisible(Uint16 inVisible)												{	UWindow::SetVisible(mWindow, inVisible);								}
		bool IsVisible() const															{	return UWindow::IsVisible(mWindow);										}
		void Show()																		{	SetVisible(true);														}
		void Hide()																		{	SetVisible(false);														}
		
		// children
		bool IsChild(CWindow *inWindow);
		bool GetChildrenList(CPtrList<CWindow>& outChildrenList);
		void SetVisibleChildren(Uint16 inVisible);
		void ShowChildren()																{	SetVisibleChildren(true);												}
		void HideChildren()																{	SetVisibleChildren(false);												}
		
		// collapsing
		void Collapse()																	{	UWindow::Collapse(mWindow);												}
		void Expand()																	{	UWindow::Expand(mWindow);												}
		bool IsCollapsed()																{	return UWindow::IsCollapsed(mWindow);									}
		bool IsZoomed()																	{	return UWindow::IsZoomed(mWindow);										}

		// background color
		void SetBackColor(const SColor& inColor)										{	UWindow::SetBackColor(mWindow, inColor);								}
		void GetBackColor(SColor& outColor) const										{	UWindow::GetBackColor(mWindow, outColor);								}
		
		// layer
		void SetLayer(Uint16 inLayer)													{	UWindow::SetLayer(mWindow, inLayer);									}
		Uint16 GetLayer() const															{	return UWindow::GetLayer(mWindow);										}
	
		// bounds
		virtual void SetBounds(const SRect& inBounds);
		void GetBounds(SRect& outBounds) const											{	UWindow::GetBounds(mWindow, outBounds);									}
		void SetLimits(Int32 inMinWidth, Int32 inMinHeight, Int32 inMaxWidth = max_Int16, Int32 inMaxHeight = max_Int16) { UWindow::SetLimits(mWindow, inMinWidth, inMinHeight, inMaxWidth, inMaxHeight); }
		bool GetAutoBounds(Uint16 inPosition, Uint16 inPosOn, SRect& outBounds) const	{	return UWindow::GetAutoBounds(mWindow, inPosition, inPosOn, outBounds);	}
		void SetAutoBounds(Uint16 inPosition, Uint16 inPosOn);
		void SetLocation(const SPoint& inTopLeft);
		
		// misc properties
		static bool IsValid(CWindow *inWindow);
		Uint16 GetStyle() const															{	return UWindow::GetStyle(mWindow);										}
		Uint16 GetOptions() const														{	return UWindow::GetOptions(mWindow);									}
		void SetType(Uint16 inType)														{	mType = inType;															}
		Uint16 GetType() const															{	return mType;															}
		Uint16 GetState() const															{	return UWindow::GetState(mWindow);										}

		// layers
		bool IsFront() const															{	return UWindow::IsFront(mWindow);										}
		bool BringToFront(bool inActivate = true)										{	return UWindow::BringToFront(mWindow, inActivate);						}
		bool InsertAfter(CWindow *inInsertAfter, bool inActivate = true);
		bool SendToBack()																{	return UWindow::SendToBack(mWindow);									}
		static CWindow *GetFocus();
		static CWindow *GetFront(Uint16 inLayer);

		// mouse capturing
		void CaptureMouse();
		void ReleaseMouse();
		bool IsMouseCaptureWindow();
		static CWindow *GetMouseCaptureWindow();
		virtual void MouseCaptureReleased();

		// drawing
		void Refresh()																	{	UWindow::Refresh(mWindow);												}
		void Refresh(const SRect& inRect)												{	UWindow::Refresh(mWindow, inRect);										}
		virtual void Draw(const SRect& inUpdateRect);
		
		// mouse
		virtual void MouseDown(const SMouseMsgData& inInfo);
		virtual void MouseUp(const SMouseMsgData& inInfo);
		virtual void MouseEnter(const SMouseMsgData& inInfo);
		virtual void MouseMove(const SMouseMsgData& inInfo);
		virtual void MouseLeave(const SMouseMsgData& inInfo);
		
		// key
		virtual void KeyDown(const SKeyMsgData& inInfo);
		virtual void KeyUp(const SKeyMsgData& inInfo);
		virtual void KeyRepeat(const SKeyMsgData& inInfo);
		
		// drag and drop
//		virtual bool CanAcceptDrop(TDrag inDrag) const;
		virtual void DragEnter(const SDragMsgData& inInfo);
		virtual void DragMove(const SDragMsgData& inInfo);
		virtual void DragLeave(const SDragMsgData& inInfo);
		virtual bool Drop(const SDragMsgData& inInfo);
		
		// QuickTime events
		virtual void SendToQuickTime(const EventRecord& inInfo);

		// timer
		TTimer NewTimer()														{	return UTimer::New(TimerHandler, this);										}
		TTimer StartNewTimer(Uint32 inMillisecs, Uint32 inIsRepeating = false)	{	return UTimer::StartNew(TimerHandler, this, inMillisecs, inIsRepeating);	}
		virtual void Timer(TTimer inTimer);

		// user
		virtual void UserClose(const SMouseMsgData& inInfo);
		virtual void UserZoom(const SMouseMsgData& inInfo);
		virtual void BoundsChanged();
		virtual void StateChanged();
		
		// hits
		virtual void Hit(const SHitMsgData& inInfo);
		bool GetHit(SHitMsgData& outInfo, Uint32 inMaxDataSize = 0);
		Uint32 GetHitID();
		Uint32 GetHitCommandID();

		// misc
		virtual void HandleMessage(void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize);
		void ProcessModal();

		// snapping
		void MakeNoSnapWin();
		void MakeNoResizeWin();
		void MakeKeepOnScreenWin();

		// typecast operators
		operator TWindow() const					{	return mWindow;					}
		
		// misc
		static Uint32 GetLastKeyMouseEventSecs()	{	return mLastKeyMouseEventSecs;	}

	protected:
		CLinkedList mHitQueue;
		TWindow mWindow;
		Uint16 mType;

		Uint16 mIsModal					: 1;
		Uint16 mIsMouseCapturedView		: 1;
//		Uint16 mDragWithinViewCanAccept	: 1;
		
		// view handlers
		virtual void HandleInstall(CView *inView);
		virtual void HandleRemove(CView *inView);
		virtual void HandleRefresh(CView *inView, const SRect& inUpdateRect);
		virtual void HandleHit(CView *inView, const SHitMsgData& inInfo);
		virtual void HandleGetScreenDelta(CView *inView, Int32& outHoriz, Int32& outVert);
		virtual void HandleGetVisibleRect(CView *inView, SRect& outRect);
		virtual void HandleCaptureMouse(CView *inView);
		virtual void HandleReleaseMouse(CView *inView);
		virtual CView *HandleGetCaptureMouseView();
		virtual CWindow *HandleGetParentWindow();
		
		// misc
		static Uint32 mLastKeyMouseEventSecs;
		static void MessageHandler(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize);
		static void TimerHandler(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize);
};


class CWindowList
{
	public:
		CWindowList();
		~CWindowList();
		
		Uint32 AddWindow(CWindow *inWindow);
		Uint32 RemoveWindow(CWindow *inWindow);

		bool GetNextWindow(CWindow*& ioWindow, Uint32& ioIndex) const	{	return mWindowList.GetNext(ioWindow, ioIndex);		}
		bool GetPrevWindow(CWindow*& ioWindow, Uint32& ioIndex) const	{	return mWindowList.GetPrev(ioWindow, ioIndex);		}
		bool IsInList(CWindow *inWindow) const							{	return mWindowList.IsInList(inWindow);				}
	
		void ClearWindowList();

	protected:
		CPtrList<CWindow> mWindowList;
};

extern CWindowList gWindowList;


class CSnapWindows
{
	public:
		CSnapWindows();
		~CSnapWindows();
	
		void EnableSnapWindows(bool inEnableDisable, Int32 inSnapSoundID = 0, Int32 inDetachSoundID = 0);
		bool IsEnableSnapWindows();
		
		void AddNoSnapWin(CWindow *inWindow);
		void AddNoResizeWin(CWindow *inWindow);
		void AddKeepOnScreenWin(CWindow *inWindow);
		void RemoveWindow(CWindow *inWindow);

		void CreateSnapWinList(CWindow *inWindow);
		void CreateSnapSound(CWindow *inWindow);
		void DestroySnapWinListSound(bool inClear = true);
		
		void WinSnapTogether(CWindow *inWindow, bool inMoveSize);
		bool WinSnapTogether(CWindow *inWindow, SRect& ioWindowBounds, bool inMoveSize);
		void WinMoveTogether(CWindow *inWindow);
		void WinMoveTogether(CWindow *inWindow, SRect inWindowBounds);
		bool WinKeepOnScreen(CWindow *inWindow, SRect& inWindowBounds);
		
	protected:
		bool RectSnapTogether(SRect& inWindowBounds, SRect inWinBounds, bool inMoveSize);
		bool RectSnapScreen(SRect& ioWindowBounds, bool inMoveSize);
		bool RectKeepOnScreen(SRect& ioWindowBounds);
		void ComposeSnapWinList(CWindow *inWindow, SRect inWindowBounds);
		void ComposeSnapSound(CWindow *inWindow, SRect inWindowBounds, bool inPlaySound = true, bool inCheckWin = true);

		bool mEnableDisable;
		Int32 mSnapSoundID;
		Int32 mDetachSoundID;

		CPtrList<CWindow> mNoSnapWinList;
		CPtrList<CWindow> mNoResizeWinList;
		CPtrList<CWindow> mScreenWinList;

		bool mBringToFront;
		CWindow *mAnchorSnapWin;
		SRect mAnchorSnapWinBounds;
		
		bool mSnapLeft, mSnapTop, mSnapRight, mSnapBottom;
		bool mSnapRightTop, mSnapLeftTop, mSnapLeftBottom, mSnapRightBottom;
		bool mScreenLeft, mScreenTop, mScreenRight, mScreenBottom;

		CWindow *mTempSnapWin;
		CPtrList<CWindow> mTempSnapWinList;		
};

extern CSnapWindows gSnapWindows;
