/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "CView.h"
#include "UIcon.h"

class CSimpleIconButtonView : public CView
{	
	public:
		CSimpleIconButtonView(CViewHandler *inHandler, const SRect& inBounds);
		
		virtual void SetIcons(TIcon inEnabled, TIcon inDisabled, TIcon inHilited);
		void SetIconIDs(Int32 inEnabledID, Int32 inDisabledID, Int32 inHilitedID);
		void SetIconID(Int32 inID)				{	SetIconIDs(inID, 0, 0);			}

		TIcon GetEnabledIcon() const			{	return mEnabledIcon;			}
		TIcon GetDisabledIcon() const			{	return mDisabledIcon;			}
		TIcon GetHilitedIcon() const			{	return mHilitedIcon;			}
		TIcon GetIcon() const					{	return mEnabledIcon;			}

		virtual bool ChangeState(Uint16 inState);
		virtual bool SetEnable(bool inEnable);
		virtual void Draw(TImage inImage, const SRect& inUpdateRect, Uint32 inDepth);
		virtual void MouseDown(const SMouseMsgData& inInfo);
		virtual void MouseUp(const SMouseMsgData& inInfo);
		virtual void MouseEnter(const SMouseMsgData& inInfo);
		virtual void MouseLeave(const SMouseMsgData& inInfo);
		virtual bool KeyDown(const SKeyMsgData& inInfo);

	protected:
		TIcon mEnabledIcon, mDisabledIcon, mHilitedIcon;
		Uint8 mIsFocused		: 1;
		Uint8 mIsHilited 		: 1;
		Uint8 mIsDrawDisabled	: 1;
		
		void UpdateActive();
};

typedef CSimpleIconButtonView CSimpleIconBtnView, CSimIconBtnView;

