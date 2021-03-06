/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "UGraphics.h"
#include "UIcon.h"

enum {
	iconBtn_Small			= 0x01,
	iconBtn_Medium			= 0x02,
	iconBtn_Large			= 0x04,
	iconBtn_TitleLeft		= 0x08,
	iconBtn_TitleOutside	= 0x10,
	
	iconBtn_MediumTitleLeft	= iconBtn_Medium | iconBtn_TitleLeft
};

struct SIconButtonInfo {
#if MACINTOSH
	TIcon icon;
#else
	THdl icon;
	Uint32 iconLayer;
#endif
	void *title;
	Uint32 titleSize;
	Uint32 encoding;
	Uint32 options;
};

class UIconButton
{
	public:
		static void Draw(TImage inImage, const SRect& inBounds, const SIconButtonInfo& inInfo);
		static void DrawFocused(TImage inImage, const SRect& inBounds, const SIconButtonInfo& inInfo);
		static void DrawHilited(TImage inImage, const SRect& inBounds, const SIconButtonInfo& inInfo);
		static void DrawDisabled(TImage inImage, const SRect& inBounds, const SIconButtonInfo& inInfo);
		static void CalcRects(TImage inImage, const SRect& inBounds, const SIconButtonInfo& inInfo, SRect *outButtonRect, SRect *outTextRect, SRect *outIconRect);
};



