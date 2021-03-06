/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "CView.h"

class CColorView : public CView
{
	public:
		// construction
		CColorView(CViewHandler *inHandler, const SRect& inBounds);
		CColorView(CViewHandler *inHandler, const SRect& inBounds, const SColor& inColor);

		// color
		virtual void SetColor(const SColor& inColor);
		void GetColor(SColor& outColor) const				{	outColor = mColor;		}
		
		// misc
		virtual bool ChangeState(Uint16 inState);
		virtual bool SetEnable(bool inEnable);
		virtual void Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth);
		virtual void MouseDown(const SMouseMsgData& inInfo);
		virtual void MouseUp(const SMouseMsgData& inInfo);
		virtual void MouseEnter(const SMouseMsgData& inInfo);
		virtual void MouseLeave(const SMouseMsgData& inInfo);
		virtual bool KeyDown(const SKeyMsgData& inInfo);

	protected:
		SColor mColor;
		Uint8 mIsFocused	: 1;
		Uint8 mIsHilited	: 1;

		// internal functions
		void UpdateActive();
};


