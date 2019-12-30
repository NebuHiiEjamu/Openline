/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "UGraphics.h"
#include "UDragAndDrop.h"
#include "UApplication.h"
#include "UTimer.h"
#include "UQuickTime.h"

// forward declare
class CWindow;
class CViewHandler;
class CView;

// auto resize options
enum {
	sizing_Fixed				= 0,
	sizing_None					= 0,
	
	sizing_LeftSticky			= 1,
	sizing_TopSticky			= 2,
	sizing_RightSticky			= 4,
	sizing_BottomSticky			= 8,
	
	sizing_FullHeight			= 16,
	sizing_FullWidth			= 32,
	
	sizing_HorizSticky			= sizing_LeftSticky + sizing_RightSticky,
	sizing_VertSticky			= sizing_TopSticky + sizing_BottomSticky,
	sizing_BottomRightSticky	= sizing_BottomSticky + sizing_RightSticky,
	sizing_Full					= sizing_FullHeight + sizing_FullWidth
};

// view hit types
enum {
	hitType_Standard			= 1,
	hitType_Alternate			= 2,
	hitType_Change				= 3,
	hitType_Drop				= 4
};

// command numbers
enum {
	cmd_OK						= 10000,
	cmd_Cancel					= 10001,
	cmd_Quit					= 10002,
	cmd_About					= 10003,

	cmd_NewDoc					= 10100,
	cmd_OpenDoc					= 10101,
	cmd_CloseDoc				= 10102,
	cmd_SaveDoc					= 10103,
	cmd_SaveAsDoc				= 10104,
	cmd_SaveCopyAsDoc			= 10105,
	cmd_RevertDoc				= 10106,
	cmd_PageSetupDoc			= 10107,
	cmd_PrintDoc				= 10108,
	
	cmd_Undo					= 10200,
	cmd_Redo					= 10201,
	cmd_Cut						= 10202,
	cmd_Copy					= 10203,
	cmd_Paste					= 10204,
	cmd_Clear					= 10205,
	cmd_SelectAll				= 10206
};

// view states
enum {
	viewState_Hidden			= 1,	// not related to visible
	viewState_Inactive			= 2,
	viewState_Active			= 3,
	viewState_Focus				= 4
};

// view hit message data
#pragma options align=packed
struct SHitMsgData {
	CView *view;
	Uint32 id;
	Uint32 cmd;
	Uint32 type;
	Uint32 part;
	Uint32 param;
	Uint16 dataSize;
	Uint8 data[];
};
#pragma options align=reset

// view class
class CView
{	
	public:
		// construction
		CView(CViewHandler *inHandler, const SRect& inBounds);
		virtual ~CView();
	
		// properties
		void SetID(Uint32 inID)								{	mID = inID;				}
		Uint32 GetID() const								{	return mID;				}
		void SetCommandID(Uint32 inID)						{	mCommandID = inID;		}
		Uint32 GetCommandID() const							{	return mCommandID;		}
		virtual void SetHandler(CViewHandler *inHandler);
		CViewHandler *GetHandler() const					{	return mHandler;		}
		virtual bool SetBounds(const SRect& inBounds);
		void GetBounds(SRect& outBounds) const				{	outBounds = mBounds;	}
		void SetSizing(Uint16 inSizing)						{	mSizing = inSizing;		}
		Uint16 GetSizing() const							{	return mSizing;			}
		void SetTooltipMsg(const Uint8 inMsg1[], const Uint8 inMsg2[] = nil);

		// visibility
		virtual bool SetVisible(bool inVisible);
		bool IsVisible() const								{	return mIsVisible;		}
		void Show()											{	SetVisible(true);		}
		void Hide()											{	SetVisible(false);		}
		
		// enable
		virtual bool SetEnable(bool inEnable);
		bool IsEnabled() const								{	return mIsEnabled;		}
		bool IsDisabled() const								{	return !mIsEnabled;		}
		void Enable()										{	SetEnable(true);		}
		void Disable()										{	SetEnable(false);		}
			
		// keyboard focus
		virtual void SetCanFocus(bool inCanFocus = true);
		bool CanFocus() const								{	return mCanFocus;		}

		// drawing
		virtual void Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth);
	
		// mouse events
		virtual void MouseDown(const SMouseMsgData& inInfo);
		virtual void MouseUp(const SMouseMsgData& inInfo);
		virtual void MouseEnter(const SMouseMsgData& inInfo);
		virtual void MouseMove(const SMouseMsgData& inInfo);
		virtual void MouseLeave(const SMouseMsgData& inInfo);
		bool IsAnyMouseBtnDown() const						{	return mIsLeftMouseBtnDown || mIsRightMouseBtnDown || mIsMiddleMouseBtnDown;	}
		bool IsMouseWithin() const							{	return mIsMouseWithin;						}
				
		// mouse capturing
		void CaptureMouse();
		void ReleaseMouse();
		virtual void MouseCaptureReleased();
		
		// key events
		virtual bool KeyDown(const SKeyMsgData& inInfo);
		virtual void KeyUp(const SKeyMsgData& inInfo);
		virtual bool KeyRepeat(const SKeyMsgData& inInfo);
		
		// drag and drop
//		virtual bool CanAcceptDrop(TDrag inDrag) const;
		virtual bool IsSelfDrag(TDrag inDrag) const;
		virtual void DragEnter(const SDragMsgData& inInfo);
		virtual void DragMove(const SDragMsgData& inInfo);
		virtual void DragLeave(const SDragMsgData& inInfo);
		virtual bool Drop(const SDragMsgData& inInfo);
		bool IsDragWithin() const							{	return mIsDragWithin;	}
		
		// QuickTime events
		virtual void UpdateQuickTime();
		virtual void SendToQuickTime(const EventRecord& inInfo);

		// timer
		TTimer NewTimer()														{	return UTimer::New(TimerHandler, this);										}
		TTimer StartNewTimer(Uint32 inMillisecs, Uint32 inIsRepeating = false)	{	return UTimer::StartNew(TimerHandler, this, inMillisecs, inIsRepeating);	}
		virtual void Timer(TTimer inTimer);

		// full size
		virtual void GetFullSize(Uint32& outWidth, Uint32& outHeight) const;
		virtual Uint32 GetFullHeight() const;
		virtual Uint32 GetFullWidth() const;
		void SetFullSize();
		void SetFullHeight();
		void SetFullWidth();

		// view state
		virtual bool ChangeState(Uint16 inState);
		virtual bool TabFocusNext();
		virtual bool TabFocusPrev();
		Uint16 GetState() const								{	return mState;														}
		bool IsFocus() const								{	return mState == viewState_Focus;									}
		bool IsActive() const								{	return mState == viewState_Active || mState == viewState_Focus;		}
		bool IsInactive() const								{	return mState == viewState_Inactive;								}
		bool IsReallyVisible() const						{	return mIsVisible && mState != viewState_Hidden;					}
		
		// misc
		virtual void SetHasChanged(bool inHasChanged);
		bool HasChanged() const								{	return mHasChanged;		}
		virtual bool IsPointWithin(const SPoint& inPt) const;
		virtual void GetRegion(TRegion outRgn) const;
		void Refresh()										{	Refresh(mBounds);		}
		void GetScreenDelta(Int32& outHoriz, Int32& outVert) const;
		void MakeRectVisible(const SRect& inRect, Uint16 inAlign = align_Inside);
		void GetVisibleRect(SRect& outRect) const;
		void Hit(Uint32 inType = hitType_Standard, Uint32 inPart = 1, Uint32 inParam = 0, const void *inData = nil, Uint32 inDataSize = 0);
		CWindow *GetParentWindow();
		
	protected:
		CViewHandler *mHandler;
		SRect mBounds;
		Uint32 mID;
		Uint32 mCommandID;
		TPtr mTooltipMsg1;
		TPtr mTooltipMsg2;
		Uint8 mSizing;
		Uint8 mState;
		Uint8 mIsLeftMouseBtnDown	: 1;
		Uint8 mIsRightMouseBtnDown	: 1;
		Uint8 mIsMiddleMouseBtnDown	: 1;
		Uint8 mIsMouseWithin		: 1;
		Uint8 mIsDragWithin			: 1;
		Uint8 mIsDragHilited		: 1;
		Uint8 mIsVisible			: 1;
		Uint8 mIsEnabled			: 1;
		Uint8 mCanFocus				: 1;
		Uint8 mHasChanged			: 1;
		Uint8 mHasMouseCapture		: 1;
				
		// misc
		virtual void Refresh(const SRect& inRect);
		static void TimerHandler(void *inContext, void *inObject, Uint32 inMsg, const void *inData, Uint32 inDataSize);
};

class CViewHandler
{
	public:
		virtual void HandleInstall(CView *inView);
		virtual void HandleRemove(CView *inView);
		virtual void HandleSetBounds(CView *inView, const SRect& inBounds);
		virtual void HandleRefresh(CView *inView, const SRect& inUpdateRect);
		virtual void HandleSetVisible(CView *inView, bool inVisible);
		virtual void HandleHit(CView *inView, const SHitMsgData& inInfo);
		virtual void HandleGetScreenDelta(CView *inView, Int32& outHoriz, Int32& outVert);
		virtual void HandleGetVisibleRect(CView *inView, SRect& outRect);
		virtual void HandleMakeRectVisible(const SRect& inRect, Uint16 inAlign);
		virtual void HandleCaptureMouse(CView *inView);
		virtual void HandleReleaseMouse(CView *inView);
		virtual void HandleGetOrigin(SPoint& outOrigin);
		virtual CView *HandleGetCaptureMouseView();
		virtual CWindow *HandleGetParentWindow();
};

