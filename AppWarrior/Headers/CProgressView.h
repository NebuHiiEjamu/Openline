/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "CView.h"

class CProgressView : public CView
{	
	public:
		CProgressView(CViewHandler *inHandler, const SRect& inBounds);
		
		virtual void SetValue(Uint32 inValue);
		Uint32 GetValue() const;
		virtual void SetMaxValue(Uint32 inValue);
		Uint32 GetMaxValue() const;
			
		virtual void Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth);

	protected:
		Uint32 mValue, mMaxValue;
};


