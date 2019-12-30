/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once

#include "graftypes.h"
#include "typedefs.h"


enum
{
	kTooltipState_Disabled				= 0,
	
	kTooltipState_Inactive				= 1,
	kTooltipState_Activating			= 2,
	kTooltipState_ActiveVisible			= 3,
	kTooltipState_ActiveHidden			= 4
};


class UTooltip
{
	public:
		static void Init();
		static void Activate(CView *inRefView, const Uint8 *inText);
		static void Hide();

		static void SetEnable(bool inEnable);
		static void Enable()												{	SetEnable(true);		}
		static void Disable()												{	SetEnable(false);		}		
		static bool IsEnabled();

		static Uint16 GetState();
};
