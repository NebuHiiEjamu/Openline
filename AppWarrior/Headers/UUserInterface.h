/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "UGraphics.h"

// standard system colors
enum {
	sysColor_Background			= 1,	// gray
	sysColor_Content			= 2,	// white
	sysColor_Light				= 3,	// light gray
	sysColor_Dark				= 4,	// dark gray
	sysColor_Frame				= 5,	// black
	sysColor_Label				= 6,	// black
	sysColor_ButtonLabel		= 7,	// black
	sysColor_Highlight			= 8,	// dark blue
	sysColor_InverseHighlight	= 9,	// white
	
	sysColor_Bkgnd				= sysColor_Background,
	sysColor_Hilite				= sysColor_Highlight,
	sysColor_InverseHilite		= sysColor_InverseHighlight
};

class UUserInterface
{
	public:
		// draw boxes
		static void DrawStandardBox(TImage inImage, const SRect& inBounds, const SColor *inFillColor = nil, bool inIsDisabled = false, bool inCanFocus = false, bool inIsFocus = false);
		static void DrawEtchedBox(TImage inImage, const SRect& inBounds, const void *inTitle = nil, Uint32 inTitleSize = 0);
		static void DrawSunkenBox(TImage inImage, const SRect& inBounds, const SColor *inColor = nil);
		static void DrawRaisedBox(TImage inImage, const SRect& inBounds, const SColor *inColor = nil);
		static void DrawBarBox(TImage inImage, const SRect& inBounds);
	
		// get system colors
		static void GetHighlightColor(SColor *outHilite, SColor *outInverse = nil);
		static bool GetSysColor(Uint32 inNum, SColor& outColor);
};


