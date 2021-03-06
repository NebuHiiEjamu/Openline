/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "CView.h"
#include "UIcon.h"

class CIconView : public CView
{	
	public:
		// construction
		CIconView(CViewHandler *inHandler, const SRect& inBounds);
		CIconView(CViewHandler *inHandler, const SRect& inBounds, Int32 inIconID);

		// properties
		virtual void SetIcon(TIcon inIcon);
		void SetIconID(Int32 inID);
		TIcon GetIcon() const;
		
		virtual void Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth);

	protected:
		TIcon mIcon;
};

