/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "CView.h"

class CLabelView : public CView
{	
	public:
		// construction
		CLabelView(CViewHandler *inHandler, const SRect& inBounds);
		CLabelView(CViewHandler *inHandler, const SRect& inBounds, const Uint8 *inText, TFontDesc inFont = nil);
		virtual ~CLabelView();
		
		// text
		virtual void SetText(const void *inText, Uint32 inSize);
		void SetText(const Uint8 *inText)						{	SetText(inText+1, inText[0]);						}
		Uint32 GetText(void *outText, Uint32 inMaxSize) const	{	return UMemory::Read(mText, 0, outText, inMaxSize);	}
		void AppendText(const void *inText, Uint32 inSize);
		void SetFont(TFontDesc inFont);
		void SetFont(const Uint8 *inName, const Uint8 *inStyle, Uint32 inSize, Uint32 inEffect = 0);
		Uint32 GetTextHeight() const;
		Uint32 GetTextSize() const								{	return mText ? UMemory::GetSize(mText) : 0;			}
		
		virtual void Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth);

	protected:
		THdl mText;
		TFontDesc mFont;
};

