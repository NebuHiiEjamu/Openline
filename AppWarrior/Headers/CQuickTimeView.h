/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "CLaunchUrlView.h"

// Note: Always a CQuickTimeView must be deleted in the parent window destructor. 


// resize options
enum {
	qtOption_ResizeMovie	= 1,	// the movie is resized to fit the view
	qtOption_ResizeView		= 2,	// the view is resized to fit the movie
	qtOption_ResizeWindow	= 3		// the window is resized to fit the movie (will resize the view as well)
};

// misc options
enum {
	qtOption_ShowController	= 0x01,
	qtOption_ShowGrowBox	= 0x02, 
	qtOption_ShowSaveAs		= 0x04,
	qtOption_ResolveGrowBox	= 0x08,	// for MAC only
	qtOption_LoopMovie		= 0x10,
	qtOption_UseBadge		= 0x20
};


class CQuickTimeView : public CLaunchUrlView
{
	public:
		// construction
		CQuickTimeView(CViewHandler *inHandler, const SRect& inBounds, Uint16 inResizeOptions = qtOption_ResizeMovie, Uint16 inOptions = qtOption_ShowController);
		virtual ~CQuickTimeView();
				
		// movie functions
		bool SelectMovie();
		bool StreamMovie(Int8 *inAddress);
		bool SetMovie(void *inData, Uint32 inDataSize, Uint32 inTypeCode);

		// movie commands
		bool StartMovie();
		bool StopMovie();
		void CloseMovie();
		bool StopStreamMovie();
		void SetMaxMovieSize(Uint32 inMaxHorizSize, Uint32 inMaxVertSize);
	
		// properties
		virtual bool SetBounds(const SRect& inBounds);
		virtual bool SetVisible(bool inVisible);
		virtual bool SetEnable(bool inEnable);
		virtual bool ChangeState(Uint16 inState);

		// QuickTime events
		virtual bool UpdateQuickTimeBounds();
		virtual void UpdateQuickTime();
		virtual void SendToQuickTime(const EventRecord& inEvent);
		virtual bool SaveMovieAs();

		// supported formats
		static bool IsSupported(Uint32 inTypeCode);
		//static bool IsNotSupported(Uint32 inTypeCode);
		bool IsVideoTrack();

		// supported formats
		static bool AreMoviesOpen()		{ return sMovieCount > 0; }
		
		// mouse events
		virtual void MouseDown(const SMouseMsgData& inInfo);
		virtual void MouseUp(const SMouseMsgData& inInfo);
		virtual void MouseEnter(const SMouseMsgData& inInfo);
		virtual void MouseMove(const SMouseMsgData& inInfo);
		virtual void MouseLeave(const SMouseMsgData& inInfo);

	protected:
		Movie mMovie;
		MovieController mController;	
		ComponentInstance mInstance;

		bool mIsVideoTrack;
		CTempFile *mTempFile;
		
		Uint16 mOptions;
		Uint16 mResizeOptions;
		Uint32 mMaxHorizSize;
		Uint32 mMaxVertSize;

		static Uint32 sMovieCount;

		bool MakeMovieFromFile(TFSRefObj* inMovieFile);
		bool MakeMovieFromUrlHandle(Handle inAddress, bool inUseComponent);
		bool MakeController(TWindow inWindow, Movie inMovie);
		bool MakeCustomMenu(EventRecord *inEvent);
		void CalculateMovieSize(Rect &inBounds, Uint32& outWidth, Uint32& outHeight);
		
		void SetWindowSize();
		void SetMovieSize(const Rect &inBounds);
		void GetMovieSize(Rect &outBounds);
		bool MovieContains(const SPoint& inLocation);

		friend pascal Boolean _QuickTimeFilterProc(MovieController inController, Int16 inAction, void *inParam, Int32 inRef);
};
