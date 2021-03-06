/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "CView.h"

class CLabelUrlView : public CView
{
	public:
		// construction
		CLabelUrlView(CViewHandler *inHandler, const SRect& inBounds);
		CLabelUrlView(CViewHandler *inHandler, const SRect& inBounds, const Uint8 *inURL);
		virtual ~CLabelUrlView();
		
		// mouse events
		virtual void MouseDown(const SMouseMsgData& inInfo);
		virtual void MouseUp(const SMouseMsgData& inInfo);
		virtual void MouseEnter(const SMouseMsgData& inInfo);
		virtual void MouseMove(const SMouseMsgData& inInfo);
		virtual void MouseLeave(const SMouseMsgData& inInfo);

		// view state
		virtual bool ChangeState(Uint16 inState);
		
		// drawing
		virtual void Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth);

		// set URL
		void SetURL(const void *inURL, Uint32 inSize);
		void SetURL(const Uint8* inURL);
		
		// get URL
		Uint32 GetURL(void *outURL, Uint32 inMaxSize);
		Uint32 GetURLSize();
		
		// launch URL
		virtual bool LaunchURL();

		// set font
		void SetFont(TFontDesc inFont, SColor& inHiliteColor);
		void SetFont(const Uint8 *inName, const Uint8 *inStyle, Uint32 inSize, Uint32 inEffect = 0);
		void SetFontSize(Uint32 inSize);

		// set colors
		void SetColor(SColor& inColor);
		void SetHiliteColor(SColor& inHiliteColor);
		void SetDisableColor(SColor& inDisableColor);

	protected:
		THdl mURL;
		TFontDesc mFont;
			
		SColor mColor;
		SColor mHiliteColor;
		SColor mDisableColor;
		
		bool mLaunched;
		
		void SetMouseLaunch();
		void SetMouseStandard();
};

